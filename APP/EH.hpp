template<typename T, typename Key_t, std::size_t PG_size>
class EH {
private:
const static std::size_t Registers_per_Bucket = PG_size/sizeof(T);
  std::string index_name;
	std::string data_name;
  std::uint32_t actual_depth = 2;

  struct Bucket {
    std::uint32_t current_size = 0;
    T arr[Registers_per_Bucket];
  };

  public:
  EH(const std::string& name) {
    index_name = "OUTPUT/"+name+"_EH_index.dat";
    data_name = "OUTPUT/"+name+"_EH_data.dat";
    std::fstream tester;
    tester.open(index_name, std::fstream::in);
    if(tester.is_open()){
      tester.seekg(0, std::ios::end);
      std::uint32_t index_count = (static_cast<std::uint32_t>(tester.tellg())+1)/sizeof(std::uint32_t);
      tester.close();
      if(index_count > 1) actual_depth = index_count;
      else{
        tester.open(index_name, std::fstream::out | std::fstream::trunc);
			  std::uint32_t temp_int = 0;
			  tester.write((char*) &temp_int, sizeof(std::uint32_t));
			  temp_int = sizeof(Bucket);
			  tester.write((char*) &temp_int, sizeof(std::uint32_t));
			  tester.close();
			  tester.open(data_name, std::fstream::out | std::fstream::trunc);
        Bucket bucket;
			  tester.write((char*) &bucket, sizeof(Bucket));
        tester.write((char*) &bucket, sizeof(Bucket));
			  tester.close();
      }
    }
		else{
			//ADD TWO EMPTY BUCKETS AND TWO EMPTY HEADERS IN INDEX FILE
			tester.close();
			tester.open(index_name, std::fstream::out);
			std::uint32_t temp_int = 0;
			tester.write((char*) &temp_int, sizeof(std::uint32_t));
			temp_int = sizeof(Bucket);
			tester.write((char*) &temp_int, sizeof(std::uint32_t));
			tester.close();
			tester.open(data_name, std::fstream::out);
      Bucket bucket;
			tester.write((char*) &bucket, sizeof(Bucket));
      tester.write((char*) &bucket, sizeof(Bucket));
			tester.close();
    }
  }

	private:
  std::uint32_t hash_function(const Key_t& key){
    return key % actual_depth;
  }


	void augment_actual_depth(const std::uint32_t& bucket_to_split_pos, const Bucket& prev_bucket) {
    std::fstream index_file(index_name, std::ios::in | std::ios::out | std::ios::binary);
    std::uint32_t pos_in_data_file;
		for (std::uint32_t i = 0; i < actual_depth; ++i) {
			index_file.seekg(i*sizeof(std::uint32_t), std::ios::beg);
			index_file.read((char*) &pos_in_data_file, sizeof(std::uint32_t));
			index_file.seekg(0, std::ios::end);
			index_file.write((char*) &pos_in_data_file, sizeof(std::uint32_t));
		}
    actual_depth += actual_depth;
		index_file.close();
		split(bucket_to_split_pos, prev_bucket);
	}

  private:
	void split(const std::uint32_t& bucket_to_split_pos, const Bucket& prev_bucket) {
		std::fstream index_file(index_name, std::ios::in | std::ios::out | std::ios::binary);
		std::fstream data_file(data_name, std::ios::in | std::ios::out | std::ios::binary);
    std::uint32_t temp;
    Bucket bucket;
		std::uint32_t num_buckets_pointing = 0;
		//ITERATE OVER ALL HASH ENTRIES AND FIND THE ONES POINTING TO bucket_to_split_pos
		for (std::uint32_t i = 0; i < actual_depth; i++) {
			index_file.seekg(i*sizeof(std::uint32_t), std::ios::beg);
			index_file.read((char*) &temp, sizeof(std::uint32_t));
			if (temp == bucket_to_split_pos) {
				//REUSE OLD BUCKET
        if(num_buckets_pointing == 0){
          data_file.seekg(temp, std::ios::beg);
				  data_file.write((char*) &bucket, sizeof(Bucket));
        }
				//CREATE NEW BUCKET AND UPDATE ENTRY
				else {
					data_file.seekg(0, std::ios::end);
					std::uint32_t pos_new_bucket = data_file.tellg();
          data_file.write((char*) &bucket, sizeof(Bucket));
					index_file.seekg(i*sizeof(std::uint32_t), std::ios::beg);
					index_file.write((char*) &pos_new_bucket, sizeof(std::uint32_t));
				}
        ++num_buckets_pointing;
			}
		}
    index_file.close();
    data_file.close();
		if (num_buckets_pointing == 1) {
			augment_actual_depth(bucket_to_split_pos, prev_bucket);
		}
		else {
      for(std::uint32_t i = 0; i < Registers_per_Bucket; ++i) add(prev_bucket.arr[i]);
    }
	}

  public:
	void add(const T& registro) {
		std::uint32_t pos = hash_function(registro.get_key());
		std::fstream file(index_name, std::ios::in | std::ios::binary);
		file.seekg(pos*sizeof(std::uint32_t), std::ios::beg);
		std::uint32_t bucket_position;
		file.read((char*) &bucket_position, sizeof(std::uint32_t));
    file.close();
		file.open(data_name, std::ios::in | std::ios::out | std::ios::binary);
    Bucket bucket;
		file.seekg(bucket_position, std::ios::beg);
    file.read((char*) &bucket, sizeof(Bucket));
    bool is_repeated = false;
    std::uint32_t i = 0;
    for(; i<bucket.current_size; ++i) if(bucket.arr[i].get_key() == registro.get_key()){is_repeated = true; break;}
    if(is_repeated){
      bucket.arr[i] = registro;
		  file.seekg(bucket_position, std::ios::beg);
		  file.write((char*) &bucket, sizeof(Bucket));
      file.close();
    }
		//CHECK IF BUCKET IS FULL
		else if (bucket.current_size == Registers_per_Bucket) {
			//SPLIT
			split(bucket_position, bucket);
      file.close();
      add(registro);
		}else{//IF NOT FULL
      bucket.arr[bucket.current_size++] = registro;
		  file.seekg(bucket_position, std::ios::beg);
		  file.write((char*) &bucket, sizeof(Bucket));
      file.close();
    }
	}

	bool search(const Key_t& key, T& empty) {
		std::uint32_t pos = hash_function(key);
		std::fstream file(index_name, std::ios::in | std::ios::binary);
		file.seekg(pos*sizeof(std::uint32_t), std::ios::beg);
		std::uint32_t bucket_position;
		file.read((char*) &bucket_position, sizeof(std::uint32_t));
        file.close();
		file.open(data_name, std::ios::in | std::ios::binary);
        Bucket bucket;
		file.seekg(bucket_position, std::ios::beg);
		file.read((char*) &bucket, sizeof(Bucket));
		for (int i = 0; i < bucket.current_size; i++) {
			if (bucket.arr[i].get_key() == key) { empty = bucket.arr[i]; return true; }
		}
		return false;
	}

	void erase(const Key_t& key) {
		std::uint32_t pos = hash_function(key);
		std::fstream file(index_name, std::ios::in | std::ios::out | std::ios::binary);
		file.seekg(pos*sizeof(std::uint32_t), std::ios::beg);
		std::uint32_t bucket_position;
		file.read((char*) &bucket_position, sizeof(std::uint32_t));
    file.close();
		file.open(data_name, std::ios::in | std::ios::out | std::ios::binary);
    Bucket bucket;
		file.seekg(bucket_position, std::ios::beg);
		file.read((char*) &bucket, sizeof(Bucket));
		if (bucket.current_size == 0) { return; }
		std::uint32_t i = 0;
		for (; i < bucket.current_size; ++i) {
			if (bucket.arr[i].get_key() == key) { break; }
		}
		if (i == bucket.current_size-1) {
			bucket.current_size--;
			file.seekg(bucket_position, std::ios::beg);
			file.write((char*) &bucket, sizeof(Bucket));
		}
		else {
			bucket.arr[i] = bucket.arr[bucket.current_size - 1];
			bucket.current_size--;
			file.seekg(bucket_position, std::ios::beg);
			file.write((char*) &bucket, sizeof(Bucket));
		}
	}

};