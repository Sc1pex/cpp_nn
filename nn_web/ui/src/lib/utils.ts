import { twMerge } from "tailwind-merge";
import { clsx } from "clsx";
import type { ClassValue } from "clsx";

export const cx = (...args: ClassValue[]) => twMerge(clsx(...args));
