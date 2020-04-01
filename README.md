# break-blktrace

This simple project aims at abusing how you use blktrace to crash the Linux
kernel, and fix issues found. Perhaps we should use this to selftests blktrace.

## Examples

Below are just some examples.

### Simple functional use cases

These are known to work:

  * break-blktrace -c 10 -d
  * break-blktrace -c 100 -d

### Breaking the kernel

These currently crash the kernel:

  * break-blktrace -c 10 -d -s
  * break-blktrace -c 10 -d -s -z 1000000

The [korg#205713](https://bugzilla.kernel.org/show_bug.cgi?id=205713) blames
debugfs and has anointed [CVE-2019-19770](https://nvd.nist.gov/vuln/detail/CVE-2019-19770)
for this issue, claiming the issue is a use-after-free on debugfs_remove()
on the parent dentry. However parent dentries are always positive, if
this were to be true, this CVE would have severe implications over many
filesystems.

An alternative root-cause has been determined thanks to this program, and
patches will soon be published.

License
-------

GPLv2
