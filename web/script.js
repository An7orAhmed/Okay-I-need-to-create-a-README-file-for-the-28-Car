// script.js

document.addEventListener('DOMContentLoaded', () => {
    const sidebarToggle = document.getElementById('sidebarToggle');
    const sidebar = document.getElementById('sidebarMenu');
    const statusMessage = document.getElementById('statusMessage');
    const resetButton = document.getElementById('resetButton');
    const map = document.getElementById('map');
    const body = document.body;
    let oldLa = 0, oldLo = 0;
  
    // Sidebar toggle functionality
    sidebarToggle.addEventListener('click', () => {
        sidebar.classList.toggle('show');
        if (sidebar.classList.contains('show')) {
            sidebar.style.marginLeft = '0';
        } else {
            sidebar.style.marginLeft = '-250px';
        }
    });
    
    // Event listener for Reset Button
      resetButton.addEventListener('click', () => {
          // Send the reset request
          fetch('api.php?la=0&lo=0&st=0')
              .then(response => response.json())
              .then(data => {
                  console.log('Reset successful:', data);
              })
              .catch(error => console.error('Error fetching data:', error));
      });
  
    function fetchData() {
        fetch('api.php')
            .then(response => response.json())
            .then(data => {
                if (data.st === "1") {
                    body.style.backgroundColor = '#f8d7da'; // Light red background
                    statusMessage.textContent = 'Device detected accident! EMERGENCY!!';
                    statusMessage.parentElement.classList.remove('bg-secondary', 'bg-success');
                    statusMessage.parentElement.classList.add('bg-danger');
                    if(oldLa !== data.la || oldLo !== data.lo) { 
                      map.src = `https://maps.google.com/maps?width=100%&height=600&hl=en&q=${data.la},${data.lo}&t=&z=14&ie=UTF8&iwloc=B&output=embed`;
                      oldLa = data.la;
                      oldLo = data.lo;
                    }
                } else if (data.st === "0") {
                    body.style.backgroundColor = '#d1e7dd'; // Light green background
                    statusMessage.textContent = 'Safely moving';
                    statusMessage.parentElement.classList.remove('bg-secondary', 'bg-danger');
                    statusMessage.parentElement.classList.add('bg-success');
                    map.src = '';
                } else {
                    body.style.backgroundColor = '#fff';
                    statusMessage.textContent = 'Unknown status';
                    statusMessage.parentElement.classList.remove('bg-secondary', 'bg-danger', 'bg-success');
                    statusMessage.parentElement.classList.add('bg-secondary');
                    map.src = '';
                }
            })
            .catch(error => console.error('Error fetching data:', error));
    }
  
    // Fetch data every second
    setInterval(fetchData, 1000);
    fetchData(); // Initial fetch
  });
  