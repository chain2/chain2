**Create A Debian Package Installer**

1. Download gitian created chain2 tar file to chain2/contrib/gitian-debian folder:

  ```
  cd chain2/contrib/gitian-debian
  wget https://github.com/chain2/chain2/releases/download/v0.11M/chain2-0.11.0-M-linux64.tar.gz
  ```

2. Execute debian installer build script:
  ```
  ./build.sh
  ```

**Test New Debian Package Installer**

1. Install newly created debian package on test debian system:

  ```
  sudo gdebi chain2-0.11M.deb
  ```

2. Verify chain2 daemon installed and started:

  ```
  sudo systemctl status chain2d
  ```

3. Add your user account to the bitcoin system group:
   
  ```
  sudo usermod -a -G bitcoin <your username>
  ```
  
4. Logout and back into your account so new group assignment takes affect.

5. Verify your username was added to the bitcoin group:

  ```
  groups
  ```

6. Test chain2-cli access:

  ```
  /usr/bin/chain2-cli -conf=/etc/chain2/bitcoin.conf getinfo
  ```
  
7. Test chain2-qt with non-conflicting IP port:
  
  ```
  chain2-qt -listen=0:8444
  ```
  
8. Uninstall chain2 without removing config file or data:

  ```
  sudo apt-install uninstall chain2
  ```

9. Uninstall chain2 AND remove config file and data:

  ```
  sudo apt-install purge chain2
  sudo rm -rf /var/lib/chain2
  ```

**Non-Interactive Installation**

The chain2 debian package uses debconf to ask the user if they want to automatically enable and start the chain2d service as part of the package installation. To skip this question for non-interactive installs the following instructions allow you to pre-answer the question. This question is only asked the first time the chain2 package is installed and only if the target system has the systemd systemctl binary present and executable.

1. Install ```debconf-utils```
 ```
 % sudo apt-get install debconf-utils
 ```

2. Pre-answer the question, ***true*** to automatically enable and start the ```chain2d``` service and ***false*** to not automatically enable and start the service during package install
 ```
 % sudo sh -c 'echo "chain2 chain2/start_service boolean false" | debconf-set-selections'
 ```
