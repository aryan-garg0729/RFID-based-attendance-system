import { toast } from "react-toastify";

function errorToast(status, message) {
  toast.error(`${status} | ${message}`);
}
export default errorToast;
