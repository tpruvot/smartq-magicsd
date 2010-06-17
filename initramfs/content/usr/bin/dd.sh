#!/bin/ash

unset parameters
ibs=512
obs=512
iconvrate=1024
oconvrate=1024

while [ -n "${1}" ] ; do
	key="${1%%=*}"
	value="${1#*=}"
	case "${key}" in
		(ibs)	ibs="${value}" ;;
		(obs)	obs="${value}" ;;
		(bs)	ibs="${value}" ; obs="${value}" ;;
	esac

	parameters="${parameters} ${1}"
	shift
done

#	BLOCKS  and  BYTES  may  be  followed by the following multiplicative suffixes: xM M, c 1, w 2, b 512, kB 1000, K 1024, MB 1000*1000, M
#	1024*1024, GB 1000*1000*1000, G 1024*1024*1024, and so on for T, P, E, Z, Y.

case "${ibs}" in
	(c)	ibs="${ibs%c}" ; iconvrate=1024 ;;
	(w)	ibs="$(( ${ibs%w} * 2))" ; iconvrate=1024 ;;
	(b)	ibs="${ibs%b}" ; iconvrate=1024 ;;
	(B)	ibs="${ibs%B}" ; iconvrate=1000 ;
		case "${ibs}" in
			(K)	ibs="$(( ${ibs%K} * 1000 ))" ;;
			(M)	ibs="$(( ${ibs%M} * 1000 * 1000 ))" ;;
			(G)	ibs="$(( ${ibs%G} * 1000 * 1000 * 1000 ))" ;;
			(T)	ibs="$(( ${ibs%T} * 1000 * 1000 * 1000 * 1000 ))" ;;
			(P)	ibs="$(( ${ibs%P} * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(E)	ibs="$(( ${ibs%E} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(Z)	ibs="$(( ${ibs%Z} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(Y)	ibs="$(( ${ibs%Y} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
		esac ;;
	(K)	ibs="$(( ${ibs%K} * 1024 ))" ; iconvrate=1024 ;;
	(M)	ibs="$(( ${ibs%M} * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(G)	ibs="$(( ${ibs%G} * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(T)	ibs="$(( ${ibs%T} * 1024 * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(P)	ibs="$(( ${ibs%P} * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(E)	ibs="$(( ${ibs%E} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(Z)	ibs="$(( ${ibs%Z} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
	(Y)	ibs="$(( ${ibs%Y} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; iconvrate=1024 ;;
esac

case "${obs}" in
	(c)	obs="${obs%c}" ; oconvrate=1024 ;;
	(w)	obs="$(( ${obs%w} * 2))" ; oconvrate=1024 ;;
	(b)	obs="${obs%b}" ; oconvrate=1024 ;;
	(B)	obs="${obs%B}" ; oconvrate=1000 ;
		case "${obs}" in
			(K)	obs="$(( ${obs%K} * 1000 ))" ;;
			(M)	obs="$(( ${obs%M} * 1000 * 1000 ))" ;;
			(G)	obs="$(( ${obs%G} * 1000 * 1000 * 1000 ))" ;;
			(T)	obs="$(( ${obs%T} * 1000 * 1000 * 1000 * 1000 ))" ;;
			(P)	obs="$(( ${obs%P} * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(E)	obs="$(( ${obs%E} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(Z)	obs="$(( ${obs%Z} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
			(Y)	obs="$(( ${obs%Y} * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 ))" ;;
		esac ;;
	(K)	obs="$(( ${obs%K} * 1024 ))" ; oconvrate=1024 ;;
	(M)	obs="$(( ${obs%M} * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(G)	obs="$(( ${obs%G} * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(T)	obs="$(( ${obs%T} * 1024 * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(P)	obs="$(( ${obs%P} * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(E)	obs="$(( ${obs%E} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(Z)	obs="$(( ${obs%Z} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
	(Y)	obs="$(( ${obs%Y} * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 ))" ; oconvrate=1024 ;;
esac

tmp="$( mktemp )"
rm -f "${tmp}"
mkfifo "${tmp}"
chmod 0600 "${tmp}"

dd ${parameters} 2> ${tmp} &
pid=${!}

SECONDS=0
while [ -d /proc/${pid} ] ; do

	sleep 1
	SECONDS=$(( ${SECONDS} + 1 ))

	kill -USR1 ${pid} > /dev/null 2> /dev/null
#114718+0 records in
#114717+0 records out
#58735104 bytes (59 MB) copied, 5.61526 seconds, 10.5 MB/s

	read line1
	read line2
	read line3
	isuff=""
	osuff=""
	ipssuff=""
	opssuff=""
	line1="${line1%+*}"
	line2="${line2%+*}"
	in="$(( ${line1} * ${ibs} ))"
	out="$(( ${line2} * ${obs} ))"
	ips="$(( ${in} / ${SECONDS} ))"
	ops="$(( ${out} / ${SECONDS} ))"
	for x in k M G T P E Z F ; do
		if [ ${iconvrate} -eq 0 ]; then
			continue
		fi
		if [ ${ips} -gt ${iconvrate} ] ; then
			ips="$(( ${ips} / ${iconvrate} ))"
			ipssuff="${x}"
			[ ${iconvrate} -eq 1000 ] && ipssuff="${ipssuff}B"
		fi
		if [ ${ops} -gt ${oconvrate} ] ; then
			ops="$(( ${ops} / ${oconvrate} ))"
			opssuff="${x}"
			[ ${oconvrate} -eq 1000 ] && opssuff="${opssuff}B"
		fi
		if [ ${in} -gt ${iconvrate} ] ; then
			in="$(( ${in} / ${iconvrate} ))"
			isuff="${x}"
			[ ${iconvrate} -eq 1000 ] && isuff="${ipssuff}B"
		fi
		if [ ${oconvrate} -eq 0 ]; then
			continue
		fi
		if [ ${out} -gt ${oconvrate} ] ; then
			out="$(( ${out} / ${oconvrate} ))"
			osuff="${x}"
		fi
	done
	echo -en "\rProgress: ${out}${osuff} (${ops}${opssuff}/sec) - Time: ${SECONDS}"
done < "${tmp}"
echo
rm -f "${tmp}"in/sh

