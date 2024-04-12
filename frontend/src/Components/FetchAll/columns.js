export const COLUMNS = [
    {
        id:'rfid',
        Header : 'RFID',
        accessor : 'rfid'
    },
    {
        Header : 'Roll Number',
        accessor : 'roll_no'
    },
    {
        Header : 'Name',
        accessor : 'name'
    },
    {
        id:'isInside',
        Header : 'Is Inside',
        accessor: d => d.isCheckedIn.toString()
    },
];