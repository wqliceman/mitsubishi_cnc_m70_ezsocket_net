
include config.mk
all:
#-C specifies the directory
#make -C signal   

#Executable files should be placed last
#make -C app      

#Use shell command 'for', shell variables use two $ symbols
	@for dir in $(BUILD_DIR); \
	do \
		make -C $$dir; \
	done


clean:
#-rf: Remove directories, force delete
	rm -rf app/link_obj app/dep nginx
	rm -rf signal/*.gch app/*.gch

