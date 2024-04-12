import { toast } from "react-toastify";

function errorToast(error) {
  toast.error(`${error.response.status} | ${error.response.data.message}`);
}
export default errorToast;
