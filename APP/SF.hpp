template<typename T, typename Key_t, std::size_t PG_size>
class SF{
  private:
  std::string main_name;
  std::string aux_name;


  struct Pointer{
    bool in_aux;
    std::uint32_t index;
  };

  struct Record{
    T data;
    bool dirty;
    Pointer next;
  };

  const static std::size_t aux_tolerance = PG_size/sizeof(Record);

  const Pointer End{false, 999999999};

  public:
  SF(const std::string& name){
    main_name = "OUTPUT/"+name+"_SF_main.dat";
    aux_name = "OUTPUT/"+name+"_SF_aux.dat";
    std::fstream tester;
    tester.open(main_name, std::fstream::in);
    if(tester.is_open()){
      tester.close();
    }else{
    tester.close();
    tester.open(main_name, std::fstream::out);
    tester.close();
    tester.open(aux_name, std::fstream::out);
    tester.close();
    }
  }

  private:

  void write_record(std::fstream& main, std::fstream& aux, const Pointer& p, const Record& r) {
    if(p.in_aux){
    aux.seekg(p.index*sizeof(Record), std::ios::beg);
    aux.write((char*) &r, sizeof(Record));
    aux<<std::flush;
    }
    else{
    main.seekg(p.index*sizeof(Record), std::ios::beg);
    main.write((char*) &r, sizeof(Record));
    main<<std::flush;
    }
  };

  Record read_record(std::fstream& main, std::fstream& aux, const Pointer& p) {
    Record r;
    if(p.in_aux){
    aux.seekg(p.index*sizeof(Record), std::ios::beg);
    aux.read((char*) &r, sizeof(Record));
    }
    else{
    main.seekg(p.index*sizeof(Record), std::ios::beg);
    main.read((char*) &r, sizeof(Record));
    }
    return r;
  };


  std::uint32_t get_size_main(std::fstream& main){
    main.seekg(0, std::ios::end);
    return (static_cast<std::uint32_t>(main.tellg()) + 1)/sizeof(Record);
  }

  std::uint32_t get_size_aux(std::fstream& aux){
    aux.seekg(0, std::ios::end);
    return (static_cast<std::uint32_t>(aux.tellg()) + 1)/sizeof(Record);
  }

  bool search_(const Key_t& key, T& empty, Pointer& record_dir){
    std::fstream main;
    std::fstream aux;
    main.open(main_name, std::fstream::in | std::fstream::binary);
    aux.open(aux_name, std::fstream::in | std::fstream::binary);

    const std::uint32_t main_size = get_size_main(main);

    if(main_size == 0){ //Si no hay registros en main
      main.close();aux.close();
      return false;
    }else{ //Si hay registros en main
      Record record; //Variable donde se carga el registro
      Pointer ptr = Pointer{false, 0}; //Dirección del registro, inicia en {main, 0}
      std::uint32_t inferior = 0, superior = main_size - 1;

      if(superior == 0){ //Si solo hay un registro en main
        record = read_record(main, aux, ptr);
      }else{ //Si hay más de un registro en main
        //Busqueda binaria
        while(inferior <= superior){
        ptr.index = inferior + (superior - inferior)/2;
        record = read_record(main, aux, ptr);
        if(key > record.data.get_key()) inferior = ptr.index + 1;
        else if(key < record.data.get_key())
        if(superior > 0) superior = ptr.index - 1; else break;
        else break;
        }
      }

      //Busqueda lineal (iterar por lista enlazada ordenada de menor a mayor)
     while(true){
        if(key > record.data.get_key()) { //Si es mayor
          if(record.next.in_aux) { //Si el siguiente está en aux, sigue
            ptr = record.next;
            record = read_record(main, aux, record.next);
          } else{ //Si el siguiente no está en aux, no está, FIN
            main.close();aux.close();
            return false;
          }
        } else if(key < record.data.get_key()) { //Si es menor, no está, FIN
          main.close();aux.close();
          return false;
        }else{ //Si no es ni menor ni mayor (es igual)
          if(record.dirty){ //Si está sucio, se ignora, FIN
            main.close();aux.close();
            return false;
          }else{ //Si no está sucio, se obtiene, FIN
            record_dir = ptr;
            empty = record.data;
            main.close();aux.close();
            return true;
          }
        }
      }
    }
  }

  public:

  bool search(const Key_t& key, T& empty){
    Pointer ptr;
    return search_(key, empty, ptr);
  }

  void erase(const Key_t& key){
    T empty;
    Pointer ptr;
    if(search_(key, empty, ptr)){
      std::fstream main;
      std::fstream aux;
      main.open(main_name, std::fstream::in | std::fstream::out | std::fstream::binary);
      aux.open(aux_name, std::fstream::in | std::fstream::out | std::fstream::binary);
      Record record = read_record(main, aux, ptr);
      record.dirty = true;
      write_record(main, aux, ptr, record);
      main.close();aux.close();
    }
  }

  std::vector<T> load(void){
    std::fstream main;
    std::fstream aux;
    main.open(main_name, std::fstream::in | std::fstream::binary);
    aux.open(aux_name, std::fstream::in | std::fstream::binary);
    std::vector<T> vec;
    std::uint32_t sz = get_size_main(main);
    Record record;
    if(sz > 0){
      record = read_record(main, aux, Pointer{false, 0});
      if(!record.dirty) vec.push_back(record.data);
      while(record.next.index != End.index){
        record = read_record(main, aux, record.next);
        if(!record.dirty) vec.push_back(record.data);
      }
    }
    main.close();aux.close();
    return vec;
  }

  private:
  void insertAll(std::vector<T> vec){
    std::fstream main;
    std::fstream aux;
    main.open(main_name, std::fstream::trunc | std::fstream::out | std::fstream::binary);
    aux.open(aux_name, std::fstream::trunc | std::fstream::out | std::fstream::binary);
    //std::sort(vec.begin(), vec.end(), [](const T& a, const T& b){return a.get_key() < b.get_key();});
    const std::uint32_t sz = vec.size();
    const std::uint32_t last = sz - 1;
    for(std::uint32_t i = 0; i<sz; ++i){
      if(i != last){
      write_record(main, aux, Pointer{false, i}, Record{vec[i], false, Pointer{false, i+1}});
      }else{
      write_record(main, aux, Pointer{false, i}, Record{vec[i], false, End});
      }
    }
    main.close();aux.close();
  }

  private:
  void evaluate_recreation(void){
    std::fstream main;
    std::fstream aux;
    main.open(main_name, std::fstream::in | std::fstream::binary);
    aux.open(aux_name, std::fstream::in | std::fstream::binary);
    std::uint32_t aux_sz = get_size_aux(aux); main.close();aux.close();
    if(aux_sz > aux_tolerance){
      std::vector<T> all_records = load();
      insertAll(all_records);
    }
  }

  public:
  void add(const T& data){
    std::fstream main;
    std::fstream aux;
    main.open(main_name, std::fstream::in | std::fstream::out | std::fstream::binary);
    aux.open(aux_name, std::fstream::in | std::fstream::out | std::fstream::binary);

    const std::uint32_t main_size = get_size_main(main);

    if(main_size == 0){ //Si no hay registros en main
      write_record(main, aux, Pointer{false, 0}, Record{data, false, End});
    }else{ //Si hay registros en main
      Record record; //Variable donde se carga el registro
      Pointer ptr{false, 0}; //Dirección del registro, inicia en {main, 0}
      std::uint32_t inferior = 0, superior = main_size - 1;

      if(superior == 0){ //Si solo hay un registro en main
        record = read_record(main, aux, ptr);
        if(data.get_key() == record.data.get_key()){
          write_record(main, aux, ptr, Record{data, false, record.next});
          main.close();aux.close();
          return;
        }
      }else{ //Si hay más de un registro en main

        while(inferior <= superior){
          ptr.index = inferior + (superior - inferior)/2;
          record = read_record(main, aux, ptr);
          if(data.get_key() > record.data.get_key()) inferior = ptr.index + 1;
          else if(data.get_key() < record.data.get_key()){
          if(superior > 0) superior = ptr.index - 1; else break;}
          else{
            write_record(main, aux, ptr, Record{data, false, record.next});
            main.close();aux.close();
            return;
          }
        }

      }

      if(record.dirty){ //Si el registro está sucio, se recicla, FIN
        write_record(main, aux, ptr, Record{data, false, record.next});
      }else{ //Si el registro no está sucio

        Record new_record;//Registro por insertar en espacio libre de aux
        const Pointer new_ptr{true, get_size_aux(aux)};//Espacio libre en aux


        if(data.get_key() < record.data.get_key()){ //Si el registro (de main) es menor
          new_record = record; //El registro de main pasa a ser el registro nuevo/suelto
          record.data = data;
          write_record(main, aux, ptr, record); //La data nueva sobreescribe la data de ese registro
        }else{ //Si el registro de main no es menor
          new_record = Record{data, false, Pointer{false, 0}}; //El registro nuevo/suelto es creado con la data nueva
        }

        if(record.next.in_aux){ //Si el siguiente está en aux

          Pointer prev_ptr = ptr;
          Record prev_record = record;
          ptr = record.next;
          record = read_record(main, aux, record.next);

          while(new_record.data.get_key() > record.data.get_key() &&record.next.in_aux) {
            prev_ptr = ptr;
            prev_record = record;
            ptr = record.next;
            record = read_record(main, aux, record.next);

          } //Se itera sobre la lista en aux hasta ue uno sea mayor

          if(new_record.data.get_key() == record.data.get_key()){
              new_record.next = record.next;
              write_record(main, aux, ptr, new_record);
              return;
          }

          if(record.dirty) { //Si ese registro está sucio, se recicla, FIN
            new_record.next = record.next;
            write_record(main, aux, ptr, new_record);
          }else{ //Si no está sucio
            if(record.next.in_aux || new_record.data.get_key() < record.data.get_key()){ //Si el siguiente está en aux, se inserta despues del registro previo (visto de otra manera, antes del actual), FIN
              new_record.next = prev_record.next;
              prev_record.next = new_ptr;
              write_record(main, aux, prev_ptr, prev_record);
              write_record(main, aux, new_ptr, new_record);
            }else{ //Si el siguiente no está en aux, se inserta despues del actual (push_back), FIN
              new_record.next = record.next;
              record.next = new_ptr;
              write_record(main, aux, ptr, record);
              write_record(main, aux, new_ptr, new_record);
            }
          }

        }else{ //Si el siguiente no está en aux, se crea como primer elemento de la lista en aux, FIN
          new_record.next = record.next;
          record.next = new_ptr;
          write_record(main, aux, ptr, record);
          write_record(main, aux, new_ptr, new_record);
        }
      }
    }
    main.close();aux.close();
    evaluate_recreation();
  }

  std::vector<T> search(const Key_t& inferior_with_key, const Key_t& superior_with_key){
    T empty;
    std::vector<T> vec;
    Pointer ptr;
    if(search_(inferior_with_key, empty, ptr)){
      std::fstream main;
      std::fstream aux;
      main.open(main_name, std::fstream::in | std::fstream::binary);
      aux.open(aux_name, std::fstream::in | std::fstream::binary);
      Record record = read_record(main, aux, ptr);
      vec.push_back(record.data);
      while(record.data.get_key() < superior_with_key){
        if(record.next.index != End.index){
          record = read_record(main, aux, record.next);
          vec.push_back(record.data);
        } else{
          vec.clear();
          break;
        }
      }
      main.close();aux.close();
      if(!vec.empty()) if(vec.back().get_key() > superior_with_key) vec.clear();
    }
    return vec;
  }
};