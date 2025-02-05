//零环代码
#include <wdm.h>
#include <ntddk.h>  
#include <Wdmsec.h>

#define MY_GUID L"a952eebb-84fc-4bd3-9708-3a8dc94f0c2c" //这段guid在实际使用中请自行调用CoCreateCuid生成
#define MY_CDO_NAME L"\\Device\\my_test_name"
#define MY_SYB_NAME L"\\??\\my_test_sybname123"

//定义内存分配标识
#define MEM_TAG 'MyTt'
char* DeviceBuffer = NULL;
int Length = -1;


//声明分发函数
NTSTATUS DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

//定义一个全局的控制设备
PDEVICE_OBJECT my_cdo = NULL;

// 提供一个 Unload 函数只是为了让这个程序能动态卸载，方便调试
VOID DriverUnload(PDRIVER_OBJECT driver)
{
    DbgPrint("first:our driver is unloading..r\n");
    UNICODE_STRING my_syb_name = RTL_CONSTANT_STRING(MY_SYB_NAME);

    // 删除符号链接
    IoDeleteSymbolicLink(&my_syb_name);

    //释放内核buffer
    if (DeviceBuffer)
    {
        ExFreePoolWithTag(DeviceBuffer, MEM_TAG);
    }

    // 删除设备对象
    if (my_cdo)
    {
        IoDeleteDevice(driver->DeviceObject);
    }
}

NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG length = irpSp->Parameters.Read.Length;

    // 检查 DeviceBuffer 是否已初始化且不为空
    if (DeviceBuffer == NULL || Length == -1)
    {
        DbgPrint("Device buffer is uninitialized!");

        Irp->IoStatus.Status = STATUS_NO_DATA_DETECTED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_NO_DATA_DETECTED;
    }

    // 检查用户缓冲区长度是否有效
    if (length == 0)
    {
        DbgPrint("Invalid read length!");

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER;
    }

    // 将数据从 DeviceBuffer 复制到用户缓冲区
    RtlCopyMemory(buffer, DeviceBuffer, Length);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = Length;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG length = irpSp->Parameters.Write.Length;

    if (DeviceBuffer == NULL || length == -1)
    {
        //为目标字符串分配内存
        DeviceBuffer = (char*)ExAllocatePoolWithTag(NonPagedPool, length, MEM_TAG);

        Length = length;
    }
    else if (length > 0x10)
    {
        DbgPrint("buffer too big");

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER;
    }

    // 假设你要将数据写入设备的内存缓冲区
    if (DeviceBuffer != NULL && buffer != NULL)
    {

        RtlCopyMemory(DeviceBuffer, buffer, length);

        Length = length;

        DbgPrint("all right");

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = Length;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }
    else
    {
        DbgPrint("Memory allocation failed!\n");

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

// 处理 IRP_MJ_CREATE 和 IRP_MJ_CLOSE 请求
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// DriverEntry，入口函数，相当于main
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
    UNREFERENCED_PARAMETER(reg_path);

    NTSTATUS status;
    //设备名定义在\Device\路径下
    UNICODE_STRING my_cdo_name = RTL_CONSTANT_STRING(MY_CDO_NAME);
    //定义万能打开权限
    UNICODE_STRING my_sddl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;WD)");

    //创建获得控制设备对象, 用这种方式创建的控制设备，三环程序可以不需要管理员权限也可以打开
    status = IoCreateDeviceSecure(
        driver,                     //设备从属的驱动对象
        0, &my_cdo_name,            //设备扩展，控制设备名字
        FILE_DEVICE_UNKNOWN,        //需要创建的设备类型，此处我们只是实现一个自己的通信设备没有实际对应的硬件模板
        FILE_DEVICE_SECURE_OPEN,    //表示系统可以检查其权限
        TRUE, &my_sddl,             //TRUE表示可以同时被多个3环程序打开，my_sddl表示万能打开权限
        (LPCGUID)&MY_GUID,          //GUID
        &my_cdo                     //返回的控制设备句柄
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrint("IoCreateDeviceSecure error");
        return status;
    }

    //设置io方式 !重要!///////////////////////////

    // 设置 DO_BUFFERED_IO 标志，启用缓冲 I/O 模式
    my_cdo->Flags |= DO_BUFFERED_IO;

    // 清除 DO_DEVICE_INITIALIZING 标志
    my_cdo->Flags &= ~DO_DEVICE_INITIALIZING;

    //为刚刚生成的控制对象绑定符号链接
    //符号链接定义在\??\下
    UNICODE_STRING my_syb_name = RTL_CONSTANT_STRING(MY_SYB_NAME);
    status = IoCreateSymbolicLink(
        &my_syb_name,                //符号链接名
        &my_cdo_name               //控制设备名
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrint("IoCreateSymbolicLink error");
        IoDeleteDevice(driver->DeviceObject);     //如果创建符号链接失败，记得关闭已经创建的控制设备
        return status;
    }

    //配置分发函数
    driver->DriverUnload = DriverUnload;
    driver->MajorFunction[IRP_MJ_READ] = DispatchRead;
    driver->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    driver->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    driver->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;

    return STATUS_SUCCESS;
}
