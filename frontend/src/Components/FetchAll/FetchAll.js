import React, { useEffect, useMemo, useState } from "react";
import axios from "axios";
import {
  useTable,
  useSortBy,
  usePagination,
  useGlobalFilter,
} from "react-table";
import { COLUMNS } from "./columns";
// import MOCK_DATA from "./MOCK_DATA.json";
import GlobalFilter from "./GlobalFilter";
import "./FetchAll.css";

const GET_URL = "http://localhost:4000/admin/fetchAll";

const FetchAll = () => {
  //memoize the data (do not refresh data after every render)
  const columns = useMemo(() => COLUMNS, []);
  // const data = useMemo(() => MOCK_DATA, []);

  const [data, setData] = useState([]);

  const fetchData = async () => {
    try {
      const response = await axios.get(GET_URL);
      setData(response.data);
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);
  
  const tableInstance = useTable(
    {
      columns: columns,
      data: data,
    },
    useGlobalFilter,
    useSortBy,
    usePagination
  );

  //destructuring tableInstance
  const {
    getTableProps,
    getTableBodyProps,
    headerGroups,
    prepareRow,
    pageOptions,
    page,
    state: { pageIndex, pageSize, globalFilter },
    gotoPage,
    previousPage,
    nextPage,
    setPageSize,
    canPreviousPage,
    canNextPage,
    pageCount,
    setGlobalFilter,
  } = tableInstance;

  // const {globalFilter} = state;
// console.log({...getTableBodyProps});
  return (
    <>
      <GlobalFilter filter={globalFilter} setFilter={setGlobalFilter} />
      <table {...getTableProps()}>
        <thead>
          {headerGroups.map((headerGroup) => (
            <tr {...headerGroup.getHeaderGroupProps()}>
              {headerGroup.headers.map((column) => (
                <th {...column.getHeaderProps(column.getSortByToggleProps())}>
                  {column.render("Header")}
                  <span>
                    {column.isSorted ? (column.isSortedDesc ? "🔽" : "🔼") : ""}
                  </span>
                </th>
              ))}
            </tr>
          ))}
        </thead>
        <tbody {...getTableBodyProps()}>
          {page.map((row) => {
            prepareRow(row);
            console.log(row);
            return (
              <tr {...row.getRowProps()}>
                {row.cells.map((cell) => {
                  return (
                    <td {...cell.getCellProps()}>{cell.render("Cell")}</td>
                  );
                })}
              </tr>
            );
          })}
        </tbody>
      </table>
      {/* NAVIGATORS */}
      <div className="navigators">
        <span className="showPageNumber">
          Page:{" "}
          <em>
            {pageIndex + 1} of {pageOptions.length + " "}
          </em>
        </span>
        <span className="gotoPage">
          | Go to Page:{" "}
          <input
            type="number"
            style={{ width: "50px" }}
            defaultValue={pageIndex + 1}
            onChange={(e) => {
              const pageNumber = e.target.value
                ? Number(e.target.value) - 1
                : 0;
              gotoPage(pageNumber);
            }}
          />
        </span>
        <span>
          {" "}
          <select
            name="#"
            id="#"
            value={pageSize}
            onChange={(e) => setPageSize(Number(e.target.value))}
          >
            {[10, 25, 50].map((pageSize) => (
              <option key={pageSize} value={pageSize}>
                Show {pageSize}
              </option>
            ))}
          </select>{" "}
        </span>
        <button
          onClick={() => gotoPage(gotoPage(0))}
          disabled={!canPreviousPage}
          title="Start Page"
        >
          {"<<"}
        </button>

        <button onClick={() => previousPage()} disabled={!canPreviousPage}>
          Previous Page
        </button>
        <button onClick={() => nextPage()} disabled={!canNextPage}>
          Next Page
        </button>
        <button
          onClick={() => gotoPage(pageCount - 1)}
          disabled={!canNextPage}
          title="End Page"
        >
          {">>"}
        </button>
      </div>
      {/* {document.body.} */}
    </>
  );
};

export default FetchAll;
