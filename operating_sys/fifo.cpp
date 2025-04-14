#include<bits/stdc++.h>
#include <time.h>
using namespace std ;

ofstream ofile;


bool pteall = false;
#define pb push_back
#define mp make_pair

int M,V,P, MM_num_pages, VM_num_pages, pid_counter;
char* main_memory;
struct PageInfo {
    int pid;  // Process ID
    int vpn;  // Virtual Page Number
};
PageInfo* main_memory_pages;  // Array to store pid and vpn for each page
//main_memory_pages = (PageInfo*) calloc(MM_num_pages, sizeof(PageInfo));


map<int,map<int,pair<int,int>>> MM_page_table, VM_page_table; // id vpn present_bit lpn 
set<int> free_MM_pages, free_VM_pages; 
set<int> active_process_ids; // proceess_ids loaded in swap and main memory 
set<string> active_process_names; // proceess_names loaded in swap and main memory 
map<int,int> process_size; // in bytes
map<int,string> process_name; // id->name(file_name)
map<string, int> process_id; // file_name -> id ;
// stack<int> recently_run_process;

vector<pair<int, pair<int,int>>>main_page_queue ;
vector<pair<int,pair<int,int>>> virtual_page_queue ;

int getPhysicalAddress(int pid, int logAdd);
void swapin_page_main(int pid, int vpn);

int getPhysicalAddress(int pid, int logAdd)
{  cout<<"we got here from process_ins"<<endl ;
  // cout<<logAdd<<endl;
   //  cout<<process_size[pid]<<endl;
	if(logAdd >= process_size[pid])
	{
		printf("Invalid Memory Address %d specified for process id %d\n",logAdd,pid);
		return -1;
	}

	   int vpn = ceil(logAdd/P);
    //   cout<<vpn<<endl;
	   if(MM_page_table[pid][vpn].first == 0){
        cout<<"we came to if_cond in physad"<<endl;
		swapin_page_main(pid, vpn);
	   }
	
	int physical_address = (MM_page_table[pid][vpn].second)*P + (logAdd)%P ;
  return physical_address;
}

void process_instructions(int pid){

    cout<<"we came to process_ins from run"<<endl;
     
	 string p_name = process_name[pid] ;
	 string command  ="";
	 int x,x1 ;int y,y1; int z,z1 ;

	 ifstream infile(p_name) ;
	 infile>>x ;

	 char comma ;
	 bool possible = true ;

    while((infile>>command) && possible){
        cout<<command<<endl;
		if(command == "add"){
			infile>>x ; infile>>comma ; infile>>y ; infile>>comma ; infile>>z ;
			x1 = getPhysicalAddress(pid,x);
			y1 = getPhysicalAddress(pid,y);
			z1 = getPhysicalAddress(pid,z) ;
			if(x1==-1 || y1==-1 || z1==-1) {possible = false;}
			else{
				main_memory[z1] = main_memory[x1] + main_memory[y1];

			//printf("Command: add %d,%d,%d; Result: Value in addr %d = %d, addr %d = %d, addr %d = %d\n",x,y,z,x,main_memory[x1],y,main_memory[y1],z,main_memory[z1]);
            ofile << "Command: add " << x << "," << y << "," << z 
         << "; Result: Value in addr " << x << " = " <<(int) main_memory[x1]
         << ", addr " << y << " = " << (int)main_memory[y1]
         << ", addr " << z << " = " << (int) main_memory[z1] << endl;

			}
		}
		else if(command == "sub"){
			infile>>x ; infile>>comma ; infile>>y ; infile>>comma ; infile>>z ;
			x1 = getPhysicalAddress(pid,x);
			y1 = getPhysicalAddress(pid,y);
			z1 = getPhysicalAddress(pid,z) ;
			if(x1==-1 || y1==-1 || z1==-1) {possible = false;}
			else{
				main_memory[z1] = main_memory[x1] - main_memory[y1];
				//printf("Command: add %d,%d,%d; Result: Value in sub %d = %d, addr %d = %d, addr %d = %d\n",x,y,z,x,main_memory[x1],y,main_memory[y1],z,main_memory[z1]);
            ofile << "Command: sub " << x << "," << y << "," << z 
         << "; Result: Value in addr " << x << " = " << (int) main_memory[x1]
         << ", addr " << y << " = " << (int) main_memory[y1]
         << ", addr " << z << " = " << (int) main_memory[z1] << endl;

			}
		}
		else if(command == "print"){
			infile>>x;
			x1 = getPhysicalAddress(pid,x);
			if(x1==-1) {possible = false;}
			else
				{
                    printf("Command: print %d; Result: Value in addr %d = %d\n",x,x,main_memory[x1]);
                  ofile << "Command: print " << x 
                    << "; Result: Value in addr " << x 
                    << " = " << (int) main_memory[x1] << "\n";

                    
                    }
		}

		else if(command == "load"){
			infile>>x;infile>>comma;infile>>y;
			y1 = getPhysicalAddress(pid,y);
			if(y1==-1) {possible = false;}
			else
			{
				main_memory[y1] = x;
				printf("Command: load %d,%d; Result: Value of %d is now stored in addr %d\n",x,y,x,y);
                ofile << "Command: load " << x << "," << y 
                << "; Result: Value of " << x << " is now stored in addr " << y << "\n";

			}
		}
	}

    infile.close();

}
void load_process(string file_name){
   // Check if the process is already loaded
    if(active_process_names.find(file_name) != active_process_names.end()){
        ofile << "Process " << file_name << " is already in memory with process id " << process_id[file_name] << endl;
        return;
    }

    // Open the file to read its size
    int file_size, pages_needed, i = 0;
    ifstream infile(file_name);

    if(!infile.is_open()){
        //printf("%s could not be loaded - file does not exist\n", file_name.c_str());
        ofile << file_name << " could not be loaded - file does not exist\n";

        return;
    }

    infile >> file_size; // Read file size
    infile.close();

    pages_needed = ceil((file_size * 1024.0) / P);  // Calculate pages needed

    // Check if sufficient memory is available (main memory + virtual memory)
    if(free_MM_pages.size() + free_VM_pages.size() >= pages_needed){
        pid_counter++;
        process_id[file_name] = pid_counter;
        process_name[pid_counter] = file_name;
        process_size[pid_counter] = file_size * 1024;
        active_process_ids.insert(pid_counter);
        active_process_names.insert(file_name);

        set<int>::iterator it;
        i = 0;

        // First, try to load all pages into main memory (MM)
        if(free_MM_pages.size() >= pages_needed){
            while(i < pages_needed){
                it = free_MM_pages.begin(); // Get free MM page

                MM_page_table[pid_counter][i].first = 1;   // Valid bit = 1
                MM_page_table[pid_counter][i].second = *it; // Store physical page number

               main_memory_pages[*it].pid = pid_counter;
                main_memory_pages[*it].vpn = i;

                cout<<"loaded_total_in_main_mem"<<pid_counter<<" "<<i<<" "<<*it<<endl;

                free_MM_pages.erase(it); // Remove page from free list

                main_page_queue.push_back({pid_counter, {i, *it}}); // Add to page queue
                i++;
            }
           //printf("%s is loaded in main memory and is assigned process id %d\n", file_name.c_str(), pid_counter);
        ofile << file_name << " is loaded in main memory and is assigned process id " << pid_counter << "\n";

        }

        // Handle mixed MM and VM pages
        else if(free_MM_pages.size() < pages_needed && free_MM_pages.size() != 0){
            while(i < pages_needed){
                if(!free_MM_pages.empty()){
                    // Use MM pages first
                    it = free_MM_pages.begin();
                    MM_page_table[pid_counter][i].first = 1;
                    MM_page_table[pid_counter][i].second = *it;
  
                    main_memory_pages[*it].pid = pid_counter;
                    main_memory_pages[*it].vpn = i;

                    cout<<"loaded_in_main_mem"<<pid_counter<<" "<<i<<" "<<*it<<endl;

                    free_MM_pages.erase(it); // Erase MM page

                    main_page_queue.push_back({pid_counter, {i, *it}}); // Add to main memory queue
                }
                else if(!free_VM_pages.empty()){
                    // Then use VM pages
                    it = free_VM_pages.begin();
                    VM_page_table[pid_counter][i].first = 1;  // Mark as valid
                    VM_page_table[pid_counter][i].second = *it;  // Store physical page number

                    cout<<"loaded_in_virtual_mem"<<pid_counter<<" "<<i<<" "<<*it<<endl;

                    free_VM_pages.erase(it);  // Erase from free VM list

                    virtual_page_queue.push_back({pid_counter, {i, *it}}); // Add to VM queue
                }
                i++;
            }
            printf("%s is loaded in both MM and VM, assigned process id %d\n", file_name.c_str(), pid_counter);
          ofile << file_name << " is loaded in both MM and VM, assigned process id " << pid_counter << "\n";

        }

        else if(free_VM_pages.size() >= pages_needed){
            while(i < pages_needed){
                it = free_VM_pages.begin();
                VM_page_table[pid_counter][i].first = 1;  // Mark as valid
                VM_page_table[pid_counter][i].second = *it;  // Store physical page number

                cout<<"loaded_total_in_virtual_mem"<<pid_counter<<" "<<i<<" "<<*it<<endl;

                free_VM_pages.erase(it);  // Erase from VM list

                virtual_page_queue.push_back({pid_counter, {i, *it}}); // Add to VM queue
                i++;
            }
            printf("%s is loaded in virtual memory and is assigned process id %d\n", file_name.c_str(), pid_counter);
           ofile << file_name << " is loaded in virtual memory and is assigned process id " << pid_counter << "\n";

        }
    }

    else {
        
        printf("%s could not be loaded - memory is full\n", file_name.c_str());
        ofile << file_name << " could not be loaded - memory is full\n";

    }
}

void list_process(){
	vector<int>main_process, virtual_process ;
	//printf("pid of processes in Main Memory :- ");
    for(int pid : active_process_ids){
        ofile<<pid<<" ";
    }
      
	 ofile<<endl;
}

// void swapout_page_from_main(int pid, int vpn) {
//     // Check if the process is active
//     if (active_process_ids.find(pid) == active_process_ids.end()) {
//         printf("Cannot swapout process - Process with pid %d is not loaded\n", pid);
//         return;
//     }

//     if (MM_page_table[pid][vpn].first != 1) {
//         printf("Process with pid %d does not have the page %d in Main Memory\n", pid, vpn);
//         return;
//     }

//     // If the page is in memory, attempt to swap it out
//     else if(MM_page_table[pid][vpn].first == 1){
//         // Check for free space in virtual memory (VM)
//         if (free_VM_pages.size() >= 1) {
//             // Get the first free page in VM
//             set<int>::iterator it = free_VM_pages.begin();
//             VM_page_table[pid][vpn].first = 1;  // Mark the page as valid in VM
//             VM_page_table[pid][vpn].second = *it;  // Assign the free VM page number

//             free_VM_pages.erase(it);  // Remove from the free VM pages list
//             virtual_page_queue.push({pid, {vpn, *it}});  // Add the swapped page to VM queue

//             printf("Process %d's page %d swapped to virtual memory\n", pid, vpn);
//         }

//         // If no space in VM, apply FIFO replacement policy
//         else if(free_VM_pages.size()==0) {
//             // Pop the front of the virtual memory queue (oldest page)
//             pair<int, pair<int, int>> replacing_virtual = virtual_page_queue.front();
//             virtual_page_queue.pop();

//             int replacing_pid = replacing_virtual.first;
//             int replacing_vpn = replacing_virtual.second.first;
//             int replacing_lpn = replacing_virtual.second.second;

//             // Invalidate the replaced page in the VM page table
//             VM_page_table[replacing_pid][replacing_vpn].first = 0; //absent
//             VM_page_table[replacing_pid][replacing_vpn].second = 0;

//             // Swap the new page to the replaced location
//             VM_page_table[pid][vpn].first = 1;
//             VM_page_table[pid][vpn].second = replacing_lpn;

//             // Push the swapped page back to the virtual page queue
//             virtual_page_queue.push({pid, {vpn, replacing_lpn}});

//             printf("Process %d's page %d swapped out, replacing process %d's page %d in VM\n", 
//                    pid, vpn, replacing_pid, replacing_vpn);
//         }

//         // Free the page from main memory
//         free_MM_pages.insert(MM_page_table[pid][vpn].second);

//         // Invalidate the page in the main memory table
//         MM_page_table[pid][vpn].first = 0;
//         MM_page_table[pid][vpn].second = 0;

//         printf("Process %d's page %d has been swapped out from main memory\n", pid, vpn);
//     }
// }

pair<int,pair<int,int>> get_next_process_page_to_swapout_main(){
    pair<int,pair<int,int>> replacing_page_main = main_page_queue[0];
    main_page_queue.erase(main_page_queue.begin()+0);

	return replacing_page_main ;
}
void swapin_page_main(int pid, int vpn) {
    cout<<"we came to swapin_page from phyad func"<<endl;
    // Check if the process is active
    if (active_process_ids.find(pid) == active_process_ids.end()) {
        printf("Cannot swapin process - Process with pid %d is not loaded\n", pid);
        return;
    }

    // Check if the page is already in main memory
    if (MM_page_table[pid][vpn].first == 1) {
        printf("Process with pid %d's page %d is already in Main Memory\n", pid, vpn);
        return;
    }

    // If page is not in main memory, attempt to swap it in so it is swap_memory
    else {
             free_VM_pages.insert(VM_page_table[pid][vpn].second); // add the swapin page lpn to free_vm_pages;

             auto it = find(virtual_page_queue.begin(), virtual_page_queue.end(),
             make_pair(pid, std::make_pair(vpn, VM_page_table[pid][vpn].second)));

              virtual_page_queue.erase(it);
  
              VM_page_table[pid][vpn].first = 0; // deactivate the page in vm 
              VM_page_table[pid][vpn].second = 0; 



              cout<<"we succesfully deactivated"<<endl;

        // Check for free space in main memory (MM)
        if (free_MM_pages.size() >= 1) {
            cout<<"there are free_pages in main"<<endl;
             // Get the first free page in MM
             set<int>::iterator it = free_MM_pages.begin();

             // Mark the page as valid in MM and assign the free MM page number
             MM_page_table[pid][vpn].first = 1;
             MM_page_table[pid][vpn].second = *it;

                   cout<<"free mm(swap) "<<pid<<" "<<vpn<<" "<<MM_page_table[pid][vpn].second<<" "<<MM_page_table[pid][vpn].first<<" "<<endl;

             // Update main memory page with the process ID and VPN
             main_memory_pages[*it].pid = pid;
             main_memory_pages[*it].vpn = vpn;

             // Add the page to the main memory queue (FIFO replacement)
             main_page_queue.push_back({pid, {vpn, *it}});

             // Remove the free page from the free MM list
             free_MM_pages.erase(it);
              printf("Process %d's page %d swapped into main memory\n", pid, vpn);
        }
          
        // If no free space in main memory, use FIFO to swap out a page
        else if (free_MM_pages.size() == 0) {
            cout<<"oh shit there are no free pages"<<endl;
            // Get the next page to swap out (FIFO)
            pair<int, pair<int, int>> replacing_page_main = get_next_process_page_to_swapout_main();
            int swap_pid = replacing_page_main.first;
            int swap_vpn = replacing_page_main.second.first;
            int swap_lpn = replacing_page_main.second.second;

          //  cout <<"sucessfully got the swap_ids"<<endl;

            // Swap out the page from main memory
           // swapout_page_from_main(swap_pid, swap_vpn);

            // Now swap the new page into the location freed by the swapout
            MM_page_table[pid][vpn].first = 1;
            MM_page_table[pid][vpn].second = swap_lpn;

            // Update main memory page with the process ID and VPN
            main_memory_pages[swap_lpn].pid = pid;
            main_memory_pages[swap_lpn].vpn = vpn;

            MM_page_table[swap_pid][swap_vpn].first = 0;
            MM_page_table[swap_pid][swap_vpn].second =0;
            
            cout<<"full mm(swap) "<<pid<<" "<<vpn<<" "<<MM_page_table[pid][vpn].second<<" "<<MM_page_table[pid][vpn].first<<" "<<endl;
            // Add the new page to the main memory queue (FIFO replacement)
            main_page_queue.push_back({pid, {vpn, swap_lpn}});

            cout<<"we updated in main_memand page_tbale"<<endl;

              set<int> :: iterator it ;
             if(free_VM_pages.size()>0){         /* add this swap page to vm*/
             cout<<"we came to add into vm"<<endl;
               it = free_VM_pages.begin();
                VM_page_table[swap_pid][swap_vpn].first = 1;
                VM_page_table[swap_pid][swap_vpn].second = *it ;
              //  cout<<"we updtaed "<<endl;

                virtual_page_queue.push_back({swap_pid, {swap_lpn , *it}});
            
                free_VM_pages.erase(it);

                cout<<swap_pid<<" "<<swap_vpn<<" "<<VM_page_table[swap_pid][swap_vpn].second<<" "<<1-VM_page_table[swap_pid][swap_vpn].first<<endl; 
             }

            printf("Process %d's page %d swapped into main memory, replacing process %d's page %d\n", pid, vpn, swap_pid, swap_vpn);
        }
    }
}

void run_process(int pid){
    cout<<"running_process"<<endl;
	process_instructions(pid);

    int pages_needed_for_pid = ceil(process_size[pid]/P);

    // for(int i =0; i<pages_needed_for_pid; i++){
    //     if(MM_page_table[pid][i].first == 1){
    //         cout << "mm "<<pid <<" "<<i<<" "<<MM_page_table[pid][i].second<<" 1"<<endl;
    //     }
    //     else if(VM_page_table[pid][i].first == 1){
    //            cout << "vm "<<pid <<" "<<i<<" "<<VM_page_table[pid][i].second<<" 0"<<endl;
    //     }
    // }
}

void kill_process(int pid){
	if(active_process_ids.find(pid) == active_process_ids.end())
		{printf("Cannot kill process - Process with pid %d is not loaded\n",pid);
         ofile<<" this process cannot be killed because it is not loaded "<<pid<<endl;}
	else{
		string pname = process_name[pid];

		active_process_ids.erase(pid);
		active_process_names.erase(pname);
		process_name.erase(pid);
		process_id.erase(pname);
		process_size.erase(pid);

		for(int i =0;i<MM_page_table[pid].size(); i++){
			if(MM_page_table[pid][i].first== 1){
				free_MM_pages.insert(MM_page_table[pid][i].second);
			}
		}

        for(int i =0; i<main_page_queue.size();){
            if(main_page_queue[i].first == pid){
                 main_page_queue.erase(main_page_queue.begin() + i);
            }
            else{ i++; }
        }

		for(int i =0;i<VM_page_table[pid].size(); i++){
           if(VM_page_table[pid][i].first == 1){
                 free_VM_pages.insert(VM_page_table[pid][i].second);
		   }
		}

        for(int i =0 ;i<virtual_page_queue.size(); i++){
            if(virtual_page_queue[i].first == pid){
                virtual_page_queue.erase(virtual_page_queue.begin() + i);
            }
            else { i++; }
        }



		VM_page_table.erase(pid);
		MM_page_table.erase(pid);
        ofile<< " process with pid "<<pid <<"killed sucessfully"<<endl;
		printf("Process with pid %d killed successfully\n",pid);
	}
}

void printMemLoc(int start, int length)
{
	if(start<0 || start+length >= M*1024){
		//fprintf(ofile ,"Requested values out of bounds of main memory\n");
        ofile <<"requested addr cannot be printed out of bounds"<<endl;
    	}	for(int i=0; i<length; i++){
            ofile<<"value of "<<start+i<<": "<< (int) main_memory[start+i]<<endl;
		//fprintf(ofile,"Value of %d: %d\n", start+i, main_memory[start+i]); 
        }
}

void print_page_table(int pid, string file_name, bool print_time, bool pteall) {
    FILE* ofile = fopen(file_name.c_str(), "a");
    
    // Check if the file opened successfully
    if (ofile == NULL) {
        perror("Error opening file");
        return;
    }

    if (print_time) {
        time_t my_time = time(NULL);
        // Print the current time without the newline in the output
        printf("%s\n", ctime(&my_time));
        fprintf(ofile, "%s\n", ctime(&my_time));
    }

    if (active_process_ids.find(pid) == active_process_ids.end()) {
        printf("Process with pid %d is not loaded\n", pid);
        fprintf(ofile, "Process with pid %d is not loaded\n", pid);
    } else {
        // printf("Page Table of process with pid %d (-1 indicates page not in Main Memory)\n", pid);
        // fprintf(ofile, "Page Table of process with pid %d (-1 indicates page not in Main Memory)\n", pid);
        // printf("Logical Page Number\tPhysical Page Number\n");
        // fprintf(ofile, "Logical Page Number\tPhysical Page Number\n");

        // for (int i = 0; i < MM_page_table[pid].size(); i++) {

        //     int present_bit = MM_page_table[pid][i].first;
        //     if(present_bit == 1){
        //     int lpn = MM_page_table[pid][i].second;
        //       if(pteall) { fprintf(ofile, "%d\t",pid) ;}
        //     fprintf(ofile, "%d\t%d\t%d\n", i, lpn, present_bit); }

            
        // }
        // // fprintf(ofile, "Page Table of process with pid %d (-1 indicates page not in v Memory)\n", pid);
        // // printf("Logical Page Number\tPhysical Page Number\n");
        // // fprintf(ofile, "Logical Page Number\tPhysical Page Number\n");

        // for(int i =0;i<VM_page_table[pid].size(); i++){
        //     int present_bit = VM_page_table[pid][i].first;
        //     int lpn = VM_page_table[pid][i].second;

        //     // Print LPN, PPN, and present bit correctly
        //     if(present_bit==1){
        //     //printf("%20d\t%20d\t%20d\n", i, lpn, 1- present_bit);
        //      if(pteall) { fprintf(ofile, "%d\t",pid) ;}
        //     fprintf(ofile, "%d\t%d\t%d\n", i, lpn,1-present_bit); }

        // }

        int pages_need_for_pid = ceil(process_size[pid]/P);

        for(int i =0;i<pages_need_for_pid; i++){
            if(MM_page_table[pid][i].first == 1){
                 fprintf(ofile, " %d %d 1\n", i, MM_page_table[pid][i].second );
            }
            else if(VM_page_table[pid][i].first == 1){
                 fprintf(ofile, "%d %d 0\n", i, VM_page_table[pid][i].second );
            }
        }
    }

    fclose(ofile);
}

void print_all_page_tables(string filename) {
    time_t my_time = time(NULL);
    FILE* ofile;
    ofile = fopen(filename.c_str(), "a");

    if (ofile == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(ofile, "%s\n", ctime(&my_time));
    printf("%s\n", ctime(&my_time));

    // for (int pid : active_process_ids) {

    //         pteall = true;
    //         print_page_table(pid, filename, false, pteall); // Change `false` to `true` if you want to print the time again
        
    // }

    pteall = false;

    for(int pid : active_process_ids){
        int pages_neeed_for_process = ceil(process_size[pid]/P);
        
        for(int i =0; i<pages_neeed_for_process; i++){
            if(MM_page_table[pid][i].first == 1){
              //  fprintf(ofile, "This page is present in main_mem\n");
                fprintf(ofile, "%d %d %d 1\n", pid, i, MM_page_table[pid][i].second ); // Use %d for integers
            }
            else if(VM_page_table[pid][i].first == 1){
             //   fprintf(ofile, "This page is present in vmem\n");
                fprintf(ofile, "%d %d %d 0\n", pid, i, VM_page_table[pid][i].second ); // Use %d for integers
            }
        }
    }

    fclose(ofile);
}


int main(int argc, char* argv[]) {
   // cout<<"hi1"<<endl;
    if (argc != 11) {
        printf("Usage: ./fifo -M <Main Memory Size in MB> -V <Virtual Memory Size in MB> -P <Page Size in KB> -i <input file> -o <output file>\n");
        return 1;
    }
  //  cout<<"hi2"<<endl;
    if (string(argv[0]) != "./fifo") {
        printf("This program should be executed with './fifo'\n");
        return 1;
    }

    M = atoi(argv[2]);
    V = atoi(argv[4]);
    P = atoi(argv[6]);

    MM_num_pages = ceil(M * 1024 / P);
    VM_num_pages = ceil(V * 1024 / P);
    cout<<MM_num_pages <<endl;

    for (int i = 0; i < MM_num_pages; i++)
        free_MM_pages.insert(i);
    for (int i = 0; i < VM_num_pages; i++)
        free_VM_pages.insert(i);

    main_memory = (char*)calloc(M * 1024, sizeof(char));
    main_memory_pages = (PageInfo*)calloc(MM_num_pages, sizeof(PageInfo));

    // Open input and output files
   // cout<<argv[8]<<endl;

    ifstream ifile(argv[8]);
    ofile.open(argv[10]);

   // cout<<"hi3"<<endl;

    if (!ifile.is_open()) {
        printf("Error opening input or output file.\n");
        return 1;
    }
   // cout<<"hi4"<<endl;

    string command, intermediate;
    while (getline(ifile, command)) {
        stringstream check1(command);
        vector<string> tokens;
        // while (getline(check1, intermediate, ' ')){
        //     tokens.push_back(intermediate);
        //     cout <<intermediate<<endl;
        // }
        while(check1 >> intermediate)
        {
            tokens.push_back(intermediate);
            cout <<"praizy"<<endl;
             cout <<intermediate<<endl;
        }

        if (tokens[0] == "load") {
            for (size_t i = 1; i < tokens.size(); i++)
                load_process(tokens[i]);
        } else if (tokens[0] == "run") {
            run_process(stoi(tokens[1]));
        } else if (tokens[0] == "kill") {
            kill_process(stoi(tokens[1]));
        } else if (tokens[0] == "listpr") {
            list_process();
        } else if (tokens[0] == "pte") {
              string file_name = tokens[2];
             ifstream checking(file_name);
             if(!checking){
                ofstream output_file(file_name);
             }
            print_page_table(stoi(tokens[1]), tokens[2], true, pteall);
        } else if (tokens[0] == "pteall") {
             string file_name = tokens[1];
             ifstream checking(file_name);
             if(!checking){
                ofstream output_file(file_name);
             }
            print_all_page_tables(tokens[1]);
        } else if (tokens[0] == "print") {
            printMemLoc(stoi(tokens[1]), stoi(tokens[2]));
        } else if (tokens[0] == "exit") {
            printf("Goodbye\n");
            break;
        } else {
            printf("Enter a valid command\n");
            cout << tokens[0]<<endl;
        }
    }
    
    cout<<"hi5"<<endl;
    ifile.close();
    ofile.close();
    free(main_memory);
    free(main_memory_pages);
    
    return 0;
}
