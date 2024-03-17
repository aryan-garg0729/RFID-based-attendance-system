import React from 'react'

const GlobalFilter = () => {
  return (
   <span>
    Search:{' '}
    <input value = {"filter" || ''} />
   </span>
  )
}

export default GlobalFilter;