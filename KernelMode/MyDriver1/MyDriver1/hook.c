//#pragma comment(linker, "/INTEGRITYCHECK")

#include "hook.h"

HANDLE protectID = (HANDLE)0;
PVOID hRegistration = NULL;

OB_PREOP_CALLBACK_STATUS PreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
    UNREFERENCED_PARAMETER(RegistrationContext);
    UNREFERENCED_PARAMETER(pOperationInformation);

    /*
        POB_PRE_OPERATION_INFORMATION �� �ִ� ����

        Operation
            OB_OPERATION_HANDLE_CREATE
                ���μ��� �Ǵ� �����忡 ���� �� �ڵ��� �����ϴ�. ��� ������ ����> CreateHandleInformation ���� ������ ������ ���� �� �ֽ��ϴ�.

            OB_OPERATION_HANDLE_DUPLICATE
                ���μ��� �Ǵ� ������ �ڵ��� �����˴ϴ�. �ߺ� ������ ���ؼ��� �Ű�����->DuplicateHandleInformation �� ��� �Ͻʽÿ�.

        KernelHandle
            �ڵ��� Ŀ�� �ڵ����� ���θ� �����ϴ� ��Ʈ. �� ����� TRUE �̸� �ڵ��� Ŀ�� �ڵ��Դϴ�. �׷��� ������ �� �ڵ��� Ŀ�� �ڵ��� �ƴմϴ�.

        Object
            �ڵ� �۾��� ����� ���μ��� �Ǵ� ������ ��ü�� ���� �������Դϴ�.

        ObjectType
            ��ü�� ��ü ������ ���� �������Դϴ�. �� ����� PsProcessType ���μ��� ���� ���� PsThreadType �����忡 ����.
        
        CallContext
            �۾��� ���� ����̹��� ���ؽ�Ʈ ������ ���� �������Դϴ�. 
            �⺻������ ���� �����ڴ� �� ����� NULL �� ���� ������ ObjectPreCallback ��ƾ�� ����̹��� ������� CallContext ����� �缳���� �� �ֽ��ϴ� . 
            ���� �����ڴ� �� ���� ��ġ�ϴ� ObjectPostCallback ��ƾ�� �����մϴ�.

        Parameters
            �۾��� ������ ���� �ϴ� OB_PRE_OPERATION_PARAMETERS ����ü�� ���� ������ �Դϴ�. ���� ����� ������ ������ ��ȿ������ ����.

            CreateHandleInformation
                ���� �ִ� �ڵ�� ���õ� ������ ���� �ϴ� OB_PRE_CREATE_HANDLE_INFORMATION �����Դϴ�.
                    OB_PRE_CREATE_HANDLE_INFORMATION
                    ACCESS_MASK DesiredAccess; <-- ���۰���
                    ACCESS_MASK OriginalDesiredAccess;

            DuplicateHandleInformation
                ���� ���� �ڵ�� ���õ� ������ ���� �ϴ� OB_PRE_DUPLICATE_HANDLE_INFORMATION �����Դϴ�.
    */

    //pOperationInformation->ObjectType    PreInfo->ObjectType == *PsProcessType
    
    HANDLE pid = PsGetProcessId((PEPROCESS)pOperationInformation->Object);
    if (protectID && pid == protectID) {


       /* if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
        {
            pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
        }*/

        pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
    }


    //DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_TRACE_LEVEL, "[+] Pre Callback Routine");
    return OB_PREOP_SUCCESS;
}

void PostCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION pOperationInformation)
{
    UNREFERENCED_PARAMETER(RegistrationContext);
    UNREFERENCED_PARAMETER(pOperationInformation);
    //DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_TRACE_LEVEL, "[+] Post Callback Routine\n");
}

void testHook() {
	// DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "testHook Start\n");



    OB_OPERATION_REGISTRATION CBOperationRegistrations[1] = { { 0 } };
    /*
        PsProcessType : ���μ��� �ڵ� ������ ���� ����
        PsThreadType : ������ �ڵ� ������ ���� ����
        ExDesktopObjectType : ����ũ�� �ڵ� ������ ���� ����
    */
    CBOperationRegistrations[0].ObjectType = PsProcessType;

    /*
        OB_OPERATION_HANDLE_CREATE : ���ο� �ڵ�(ObjectType�� ����)�� �����ǰų� ������ ��� ����
        OB_OPERATION_HANDLE_DUPLICATE : ���ο� �ڵ��� �����ϰų� ������ ��� ����
    */
    CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
    CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;

    /*
        OB_PRE_OPERATION_CALLBACK�� ������, ��û�� �۾��� �߻��ϱ� ���� �ش� ��ƾ�� ȣ��
    */
    CBOperationRegistrations[0].PreOperation = PreCallback;

    /*
        PostOperation : OB_POST_OPERATION_CALLBACK�� ������, ��û�� �۾��� �߻��� �Ŀ� �ش� ��ƾ�� ȣ��
    */
    //CBOperationRegistrations[0].PostOperation = PostCallback;


    OB_CALLBACK_REGISTRATION  CBObRegistration = { 0 };
    {
        CBObRegistration.Version = OB_FLT_REGISTRATION_VERSION;
        // OperationRegistration  �迭�� ��
        CBObRegistration.OperationRegistrationCount = 1;

        // ����̹��� ��(�����ڵ�), MS�� ��ϵǾ� ���Ǹ� �ε� �����͵� ���谡 ����
        UNICODE_STRING CBAltitude = { 0 };
        RtlInitUnicodeString(&CBAltitude, L"1000");
        CBObRegistration.Altitude = CBAltitude;

        // �ݹ� ��ƾ�� ����� �� �ش� ���� �ݹ� ��ƾ���� ����
        CBObRegistration.RegistrationContext = NULL;

        // OB_OPERATION_REGISTRATION�� ������, ObjectPre, PostCallback ��ƾ�� ȣ��Ǵ� ������ ����
        CBObRegistration.OperationRegistration = CBOperationRegistrations;
    }

    //PVOID pCBRegistrationHandle = NULL;
    NTSTATUS Status = ObRegisterCallbacks(
        &CBObRegistration,
        &hRegistration       // save the registration handle to remove callbacks later
    );

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "ObRegisterCallbacks : %08X\n", Status);
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "hook End\n");
}

void unhook() {
    if (hRegistration) {
        ObUnRegisterCallbacks(hRegistration);
    }
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "unhook\n");
}

void setProcessId(unsigned int pid) {
    protectID = (HANDLE)pid;
}