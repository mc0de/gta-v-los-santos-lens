#pragma once
// Generated from DurtyFree/gta-v-data-dumps vehicles.json: model hash -> in-game make,
// plus the real-life brand the make parodies (gta5wiki.com manufacturers list and gta-xtreme.com
// "GTA 5 car brands in real life"). Best guesses, not named by either source: Nagasaki (Kawasaki),
// Eberhard (Airbus), HVY (Caterpillar / Oshkosh), Kraken (Triton), LCC (Orange County Choppers),
// Buckingham planes (Gulfstream / Learjet), Western planes (Cessna). Sorted by model for binary search.
// Vehicles without a make (emergency, trains, ...) or newer than the dump aren't listed.
#include <cstdint>

struct VehicleMake { uint32_t model; const char* make; const char* real; };

static const VehicleMake kVehicleMakes[] = {
    { 0x00675ED7u, "Nagasaki", "Kawasaki" }, // chimera
    { 0x00ABB0C0u, "Nagasaki", "Kawasaki" }, // carbonrs
    { 0x00E827DEu, "Grotti", "Ferrari" }, // brioso3
    { 0x00E83C17u, "Albany", "Cadillac" }, // hermes
    { 0x00FDFFB0u, "Dundreary", "Mercury / Lincoln" }, // virgo3
    { 0x0239E390u, "Vapid", "Ford" }, // hotknife
    { 0x026ED430u, "Bravado", "Dodge" }, // greenwood
    { 0x02E19879u, "Brute", "GMC" }, // TipTruck
    { 0x0350D1ABu, "Pegassi", "Lamborghini" }, // faggio2
    { 0x03E5F6B8u, "Bravado", "Dodge" }, // youga
    { 0x0409D787u, "Declasse", "Chevrolet" }, // yosemite3
    { 0x047A6BC1u, "Benefactor", "Mercedes-Benz" }, // glendale
    { 0x04CE68ACu, "Vapid", "Ford" }, // Dominator
    { 0x04F48FC4u, "Ubermacht", "BMW" }, // rebla
    { 0x05071B2Du, "Western", "Cessna" }, // duster2
    { 0x05283265u, "Nagasaki", "Kawasaki" }, // bf400
    { 0x05852838u, "Canis", "Jeep" }, // kalahari
    { 0x0612F4B6u, "Vapid", "Ford" }, // trophytruck
    { 0x067BC037u, "Invetero", "Corvette" }, // coquette
    { 0x06FF6914u, "Albany", "Cadillac" }, // btype
    { 0x07405E08u, "Brute", "GMC" }, // boxville3
    { 0x0796B7A5u, "Invetero", "Corvette" }, // coquette6
    { 0x08852855u, "Gallivanter", "Land Rover" }, // baller2
    { 0x097E5533u, "Ocelot", "Jaguar" }, // ardent
    { 0x09D80F93u, "Buckingham", "Gulfstream / Learjet" }, // Miljet
    { 0x09E478B3u, "Bravado", "Dodge" }, // buffalo5
    { 0x09FC300Du, "Canis", "Jeep" }, // terminus
    { 0x0A90ED5Cu, "JoBuilt", "Peterbilt" }, // phantom3
    { 0x0B706A72u, "Mammoth", "Hummer" }, // streamer216
    { 0x0BBA2261u, "Annis", "Nissan" }, // elegy
    { 0x0D17099Du, "Vapid", "Ford" }, // speedo4
    { 0x0D4E5F4Du, "Grotti", "Ferrari" }, // cheetah2
    { 0x0D4EA603u, "Declasse", "Chevrolet" }, // sabregt2
    { 0x0DC60D2Bu, "Pegassi", "Lamborghini" }, // speeder
    { 0x0DF381E5u, "Pegassi", "Lamborghini" }, // reaper
    { 0x0E2C013Eu, "Bravado", "Dodge" }, // buffalo3
    { 0x0F626001u, "Ubermacht", "BMW" }, // driftcypher
    { 0x1004EDA4u, "Declasse", "Chevrolet" }, // tulip2
    { 0x1044926Fu, "Pegassi", "Lamborghini" }, // tempesta
    { 0x10635A0Eu, "Obey", "Audi" }, // tenf2
    { 0x107F392Cu, "Nagasaki", "Kawasaki" }, // dinghy2
    { 0x11291A17u, "Bravado", "Dodge" }, // polbuffalo
    { 0x1149422Fu, "Shitzu", "Suzuki" }, // tropic
    { 0x11AA0E14u, "Declasse", "Chevrolet" }, // gburrito2
    { 0x11CBC051u, "Dinka", "Honda" }, // verus
    { 0x11F58A5Au, "Nagasaki", "Kawasaki" }, // Stryder
    { 0x11F76C14u, "LCC", "Orange County Choppers" }, // hexer
    { 0x127E90D5u, "Weeny", "Mini" }, // Dynasty
    { 0x1324E960u, "Enus", "Rolls-Royce / Bentley" }, // stafford
    { 0x132D5A1Au, "Canis", "Jeep" }, // CRUSADER
    { 0x13B57D8Au, "Enus", "Rolls-Royce / Bentley" }, // cogcabrio
    { 0x142E0DC3u, "Pegassi", "Lamborghini" }, // vacca
    { 0x1446590Au, "Progen", "McLaren" }, // formula
    { 0x14D22159u, "Bravado", "Dodge" }, // gauntlet2
    { 0x14D69010u, "Vapid", "Ford" }, // chino
    { 0x1573422Du, "Gallivanter", "Land Rover" }, // baller7
    { 0x1635C007u, "Declasse", "Chevrolet" }, // vigero3
    { 0x163F8520u, "Vapid", "Ford" }, // slamvan5
    { 0x16E478C1u, "Benefactor", "Mercedes-Benz" }, // Surano
    { 0x170341C2u, "Coil", "Tesla" }, // cyclone2
    { 0x171C92C4u, "JoBuilt", "Peterbilt" }, // Hauler2
    { 0x17420102u, "Western", "Harley-Davidson" }, // cliffhanger
    { 0x177DA45Cu, "Dewbauchee", "Aston Martin" }, // jb7002
    { 0x17DF5EC2u, "Shitzu", "Suzuki" }, // squalo
    { 0x185484E1u, "Grotti", "Ferrari" }, // turismor
    { 0x185E2FF3u, "Nagasaki", "Kawasaki" }, // outlaw
    { 0x18606535u, "Mammoth", "Hummer" }, // avenger2
    { 0x18619B7Eu, "Dinka", "Honda" }, // kanjo
    { 0x187D938Du, "Karin", "Toyota" }, // kuruma2
    { 0x18F25AC7u, "Pegassi", "Lamborghini" }, // infernus
    { 0x196F9418u, "Vapid", "Ford" }, // dominator7
    { 0x19DD9ED1u, "HVY", "Caterpillar / Oshkosh" }, // nightshark
    { 0x1A144F2Au, "Pegassi", "Lamborghini" }, // speeder2
    { 0x1A79847Au, "Brute", "GMC" }, // boxville4
    { 0x1A861243u, "Vapid", "Ford" }, // imperator
    { 0x1B49B0BCu, "Dinka", "Honda" }, // driftchavosv6
    { 0x1B8165D3u, "Enus", "Rolls-Royce / Bentley" }, // jubilee
    { 0x1BB290BCu, "Declasse", "Chevrolet" }, // tornado
    { 0x1BF8D381u, "Declasse", "Chevrolet" }, // lguard
    { 0x1C09CF5Eu, "Gallivanter", "Land Rover" }, // baller5
    { 0x1C18FCE2u, "Grotti", "Ferrari" }, // cheetah3
    { 0x1C534995u, "HVY", "Caterpillar / Oshkosh" }, // Mixer2
    { 0x1CBDC10Bu, "Ocelot", "Jaguar" }, // lynx
    { 0x1D06D681u, "Enus", "Rolls-Royce / Bentley" }, // huntley
    { 0x1DC0BA53u, "Schyster", "Chrysler" }, // fusilade
    { 0x1DD4C0FFu, "Ocelot", "Jaguar" }, // swinger
    { 0x1E41B223u, "Karin", "Toyota" }, // vivanite2
    { 0x1E5E54EAu, "Nagasaki", "Kawasaki" }, // dinghy3
    { 0x1F3766E3u, "Declasse", "Chevrolet" }, // voodoo2
    { 0x1F52A43Fu, "Declasse", "Chevrolet" }, // moonbeam
    { 0x20314B42u, "Annis", "Nissan" }, // zr380
    { 0x206D1B68u, "Hijak", "Fisker" }, // khamelion
    { 0x210B9AEDu, "Karin", "Toyota" }, // everon3
    { 0x2189D250u, "HVY", "Caterpillar / Oshkosh" }, // apc
    { 0x21EEE87Du, "MTL", "Mack" }, // Packer
    { 0x2290C50Au, "Vulcar", "Volvo" }, // warrener2
    { 0x23CA25F2u, "Vapid", "Ford" }, // hustler
    { 0x24626C26u, "Penaud", "Renault" }, // coureur
    { 0x24CE9073u, "Bravado", "Dodge" }, // polbuffalo6
    { 0x250B0C5Eu, "Buckingham", "Gulfstream / Learjet" }, // luxor
    { 0x2560B2FCu, "Chariot", "Federal Coach" }, // romero
    { 0x25676EAFu, "Pegassi", "Lamborghini" }, // fcr
    { 0x256E92BAu, "Weeny", "Mini" }, // issi4
    { 0x258C9364u, "Pfister", "Porsche" }, // astron
    { 0x25C5AF13u, "Bravado", "Dodge" }, // banshee2
    { 0x25CBE2E2u, "Gallivanter", "Land Rover" }, // baller4
    { 0x26321E67u, "Principe", "Ducati" }, // lectro
    { 0x2714AA93u, "Overflod", "Koenigsegg" }, // zeno
    { 0x276D98A3u, "Pfister", "Porsche" }, // comet5
    { 0x27816B7Eu, "Obey", "Audi" }, // iwagen
    { 0x27B4E6B0u, "Gallivanter", "Land Rover" }, // baller6
    { 0x27D79225u, "Benefactor", "Mercedes-Benz" }, // bruiser
    { 0x27E34161u, "Ocelot", "Jaguar" }, // virtue
    { 0x287FA449u, "MTL", "Mack" }, // cerberus2
    { 0x28B67ACAu, "Vapid", "Ford" }, // contender
    { 0x28EAB80Fu, "Obey", "Audi" }, // drafter
    { 0x28FC5B78u, "Overflod", "Koenigsegg" }, // suzume
    { 0x293215F3u, "Declasse", "Chevrolet" }, // l352
    { 0x29B0DA97u, "BF", "Volkswagen" }, // SURFER
    { 0x29FCD3E4u, "Enus", "Rolls-Royce / Bentley" }, // cog552
    { 0x2A54C47Du, "Buckingham", "Bell" }, // supervolito
    { 0x2AE524A8u, "Hijak", "Fisker" }, // ruston
    { 0x2B0C4DCDu, "Bravado", "Dodge" }, // gauntlet3
    { 0x2B26F456u, "Imponte", "Pontiac" }, // dukes
    { 0x2B69BEA7u, "Vapid", "Ford" }, // benson2
    { 0x2B6DC64Au, "Vapid", "Ford" }, // speedo2
    { 0x2B7F9DE3u, "Vapid", "Ford" }, // slamvan
    { 0x2BC345D1u, "Vapid", "Ford" }, // sadler2
    { 0x2BE8B90Au, "Vapid", "Ford" }, // dominator8
    { 0x2BEC3CBEu, "Bravado", "Dodge" }, // buffalo2
    { 0x2C1FEA99u, "Maxwell", "Vauxhall / Opel" }, // vagrant
    { 0x2C2C2324u, "Western", "Harley-Davidson" }, // gargoyle
    { 0x2C33B46Eu, "Declasse", "Chevrolet" }, // pRanger
    { 0x2C509634u, "Western", "Harley-Davidson" }, // sovereign
    { 0x2D3BD401u, "Truffade", "Bugatti" }, // Ztype
    { 0x2DB8D1AAu, "Albany", "Cadillac" }, // alpha
    { 0x2E3967B0u, "Benefactor", "Mercedes-Benz" }, // sm722
    { 0x2E5AFD37u, "Imponte", "Pontiac" }, // ruiner3
    { 0x2EC385FEu, "Invetero", "Corvette" }, // coquette3
    { 0x2EF89E46u, "Maibatsu", "Mitsubishi" }, // Sanchez
    { 0x2FDA9E05u, "Maibatsu", "Mitsubishi" }, // monstrociti
    { 0x30D3F6D8u, "Emperor", "Lexus" }, // sheava
    { 0x30F15228u, "Annis", "Nissan" }, // drifteuros
    { 0x30FF0190u, "Shitzu", "Suzuki" }, // defiler
    { 0x31ADBBFCu, "Vapid", "Ford" }, // slamvan2
    { 0x3201DD49u, "Karin", "Toyota" }, // z190
    { 0x32174AFCu, "Bravado", "Dodge" }, // monster4
    { 0x322CF98Fu, "Declasse", "Chevrolet" }, // rhapsody
    { 0x32B29A4Bu, "Karin", "Toyota" }, // BjXL
    { 0x32B91AE8u, "HVY", "Caterpillar / Oshkosh" }, // Biff
    { 0x3329757Eu, "Eberhard", "Airbus" }, // titan2
    { 0x33581161u, "Shitzu", "Suzuki" }, // jetmax
    { 0x33B47F96u, "Nagasaki", "Kawasaki" }, // dinghy4
    { 0x33B98FE2u, "Ocelot", "Jaguar" }, // pariah
    { 0x3404691Cu, "Karin", "Toyota" }, // sultan2
    { 0x3412AE2Du, "Ubermacht", "BMW" }, // sentinel2
    { 0x34B7390Fu, "Emperor", "Lexus" }, // habanero
    { 0x34B82784u, "Pegassi", "Lamborghini" }, // oppressor
    { 0x34DBA661u, "Ocelot", "Jaguar" }, // stromberg
    { 0x34DD8AA1u, "Karin", "Toyota" }, // intruder
    { 0x353B561Du, "Declasse", "Chevrolet" }, // Burrito4
    { 0x35460148u, "Grotti", "Ferrari" }, // itali2
    { 0x35DED0DDu, "Annis", "Nissan" }, // savestra
    { 0x35ED670Bu, "Maibatsu", "Mitsubishi" }, // Mule
    { 0x360A438Eu, "Enus", "Rolls-Royce / Bentley" }, // cog55
    { 0x362CAC6Du, "Lampadati", "Maserati" }, // toro2
    { 0x36848602u, "Canis", "Jeep" }, // MESA
    { 0x36A167E0u, "Western", "Harley-Davidson" }, // rrocket
    { 0x36B4A8A9u, "Ocelot", "Jaguar" }, // xa21
    { 0x378236E1u, "Weeny", "Mini" }, // issi3
    { 0x381E10BDu, "Imponte", "Pontiac" }, // ruiner2
    { 0x3822BDFEu, "Lampadati", "Maserati" }, // casco
    { 0x38408341u, "Brute", "GMC" }, // pony2
    { 0x39085F47u, "Pegassi", "Lamborghini" }, // ignus2
    { 0x3944D5A0u, "Grotti", "Ferrari" }, // furia
    { 0x39D6E83Fu, "Mammoth", "Hummer" }, // hydra
    { 0x39DA2754u, "Karin", "Toyota" }, // sultan
    { 0x39F9C898u, "Declasse", "Chevrolet" }, // tampa
    { 0x3ADB9758u, "Dinka", "Honda" }, // Sugoi
    { 0x3AF76F4Au, "Coil", "Tesla" }, // voltic2
    { 0x3AF8C345u, "Vapid", "Ford" }, // sandking2
    { 0x3B639C8Du, "Karin", "Toyota" }, // boor
    { 0x3C26BD0Cu, "Declasse", "Chevrolet" }, // impaler2
    { 0x3C4E2113u, "Invetero", "Corvette" }, // coquette2
    { 0x3D29CD2Bu, "Bravado", "Dodge" }, // youga2
    { 0x3D7C6410u, "Pegassi", "Lamborghini" }, // tezeract
    { 0x3D8FA25Cu, "Obey", "Audi" }, // ninef
    { 0x3D961290u, "Nagasaki", "Kawasaki" }, // Dinghy
    { 0x3DA47243u, "Truffade", "Bugatti" }, // nero
    { 0x3DEE5EDAu, "Dinka", "Honda" }, // blista2
    { 0x3E2E4F8Au, "Mammoth", "Hummer" }, // tula
    { 0x3E3D1F59u, "Truffade", "Bugatti" }, // thrax
    { 0x3E5BD8D9u, "Lampadati", "Maserati" }, // michelli
    { 0x3EAB5555u, "Dewbauchee", "Aston Martin" }, // jb700
    { 0x3FC5D440u, "Vapid", "Ford" }, // bobcatXL
    { 0x3FD5AA2Fu, "Lampadati", "Maserati" }, // toro
    { 0x4008EABBu, "HVY", "Caterpillar / Oshkosh" }, // BARRACKS2
    { 0x400F5147u, "Dewbauchee", "Aston Martin" }, // SPECTER2
    { 0x4019CB4Cu, "Buckingham", "Bell" }, // swift2
    { 0x402586F8u, "Annis", "Nissan" }, // r300
    { 0x404B6381u, "Lampadati", "Maserati" }, // pigalle
    { 0x40C332A3u, "Maibatsu", "Mitsubishi" }, // manchez2
    { 0x40F546B0u, "Brute", "GMC" }, // stockade4
    { 0x4131F378u, "Truffade", "Bugatti" }, // nero2
    { 0x41B77FA4u, "Bravado", "Dodge" }, // verlierer2
    { 0x41D149AAu, "Ubermacht", "BMW" }, // sentinel3
    { 0x4201A843u, "Vapid", "Ford" }, // peyote3
    { 0x42836BE5u, "Declasse", "Chevrolet" }, // hotring
    { 0x42ACA95Fu, "Maxwell", "Vauxhall / Opel" }, // asbo
    { 0x42BC5E19u, "Vapid", "Ford" }, // slamvan3
    { 0x42D623C7u, "Bollokan", "Hyundai" }, // envisage
    { 0x42F2ED16u, "Enus", "Rolls-Royce / Bentley" }, // superd
    { 0x432AA566u, "BF", "Volkswagen" }, // BfInjection
    { 0x437CF2A0u, "Declasse", "Chevrolet" }, // burrito5
    { 0x438F6593u, "Karin", "Toyota" }, // s95
    { 0x440851D8u, "Pfister", "Porsche" }, // comet7
    { 0x4543B74Du, "Bravado", "Dodge" }, // rumpo
    { 0x462FE277u, "Benefactor", "Mercedes-Benz" }, // dubsta
    { 0x4662BCBBu, "Karin", "Toyota" }, // technical2
    { 0x4669D038u, "Declasse", "Chevrolet" }, // openwheel2
    { 0x47BBCF2Eu, "Benefactor", "Mercedes-Benz" }, // xls
    { 0x48CECED3u, "Canis", "Jeep" }, // Seminole
    { 0x49863E9Cu, "Cheval", "Holden" }, // marshall
    { 0x4992196Cu, "Progen", "McLaren" }, // gp1
    { 0x49E25BA1u, "Weeny", "Mini" }, // issi6
    { 0x4A78B890u, "Declasse", "Chevrolet" }, // polimpaler5
    { 0x4ABEBF23u, "Vapid", "Ford" }, // caracara
    { 0x4B6C568Au, "Shitzu", "Suzuki" }, // hakuchou
    { 0x4BA4E8DCu, "Dundreary", "Mercury / Lincoln" }, // landstalker
    { 0x4BFCF28Bu, "Grotti", "Ferrari" }, // bestiagts
    { 0x4C3FFF49u, "Schyster", "Chrysler" }, // deviant
    { 0x4C8DBA51u, "RUNE", "Lada" }, // zhaba
    { 0x4DC079D7u, "Pfister", "Porsche" }, // growler
    { 0x4DF2780Fu, "Canis", "Jeep" }, // castigator
    { 0x4EE74355u, "Progen", "McLaren" }, // emerus
    { 0x4FA9970Bu, "Bravado", "Dodge" }, // gauntlet6
    { 0x4FAF0D70u, "RUNE", "Lada" }, // kosatka
    { 0x4FB1A214u, "Benefactor", "Mercedes-Benz" }, // serrano
    { 0x4FF77E37u, "Buckingham", "Gulfstream / Learjet" }, // vestra
    { 0x501AC93Cu, "Maibatsu", "Mitsubishi" }, // mule5
    { 0x506434F6u, "Ubermacht", "BMW" }, // oracle
    { 0x50732C82u, "Ubermacht", "BMW" }, // sentinel
    { 0x5097F589u, "Ubermacht", "BMW" }, // sc1
    { 0x50A6FB9Cu, "Nagasaki", "Kawasaki" }, // shinobi
    { 0x50B0215Au, "MTL", "Mack" }, // FLATBED
    { 0x50D4D19Fu, "Karin", "Toyota" }, // technical3
    { 0x51D83328u, "Vulcar", "Volvo" }, // warrener
    { 0x5216AD5Eu, "Annis", "Nissan" }, // remus
    { 0x524A05A4u, "Grotti", "Ferrari" }, // gt750
    { 0x5285D628u, "Maibatsu", "Mitsubishi" }, // manchez3
    { 0x52FF9437u, "Coil", "Tesla" }, // cyclone
    { 0x546D8EEEu, "Enus", "Rolls-Royce / Bentley" }, // paragon2
    { 0x546DA331u, "Karin", "Toyota" }, // previon
    { 0x5502626Cu, "Vapid", "Ford" }, // fmj
    { 0x55365079u, "Grotti", "Ferrari" }, // brioso2
    { 0x562A97BDu, "Stanley", "John Deere" }, // tractor3
    { 0x5649FF41u, "Grotti", "Ferrari" }, // stingertt
    { 0x56590FE9u, "Shitzu", "Suzuki" }, // tropic2
    { 0x568BD0B6u, "Declasse", "Chevrolet" }, // polimpaler6
    { 0x56C8A5EFu, "Pegassi", "Lamborghini" }, // toreador
    { 0x56CDEE7Du, "Albany", "Cadillac" }, // vstr
    { 0x56D42971u, "Declasse", "Chevrolet" }, // tulip
    { 0x57F682AFu, "Bravado", "Dodge" }, // rumpo3
    { 0x586765FBu, "Imponte", "Pontiac" }, // deluxo
    { 0x5882160Fu, "Dinka", "Honda" }, // jester5
    { 0x589A840Cu, "Vapid", "Ford" }, // youga4
    { 0x58B3979Cu, "Bravado", "Dodge" }, // paradise
    { 0x58CDAF30u, "Mammoth", "Hummer" }, // thruster
    { 0x58CF185Cu, "Benefactor", "Mercedes-Benz" }, // schafter4
    { 0x58E316C7u, "LCC", "Orange County Choppers" }, // sanctus
    { 0x58E49664u, "HVY", "Caterpillar / Oshkosh" }, // FORKLIFT
    { 0x58F77553u, "Benefactor", "Mercedes-Benz" }, // openwheel1
    { 0x59A9E570u, "Pegassi", "Lamborghini" }, // torero
    { 0x59E0FBF3u, "Cheval", "Holden" }, // picador
    { 0x5A82F9AEu, "JoBuilt", "Peterbilt" }, // Hauler
    { 0x5B42A5C4u, "Declasse", "Chevrolet" }, // tornado2
    { 0x5B531351u, "Enus", "Rolls-Royce / Bentley" }, // deity
    { 0x5B73F5B7u, "Vapid", "Ford" }, // uranus
    { 0x5BA0FF1Eu, "Weeny", "Mini" }, // issi5
    { 0x5BEB3CE0u, "HVY", "Caterpillar / Oshkosh" }, // scarab2
    { 0x5C23AF9Bu, "Grotti", "Ferrari" }, // stinger
    { 0x5C54030Cu, "Imponte", "Pontiac" }, // arbitergt
    { 0x5C55CB39u, "Grotti", "Ferrari" }, // brioso
    { 0x5C6C00B4u, "Weeny", "Mini" }, // issi8
    { 0x5D1903F9u, "Pfister", "Porsche" }, // comet4
    { 0x5E2B66BEu, "Vapid", "Ford" }, // dominator10
    { 0x5E315A37u, "Annis", "Nissan" }, // hardy
    { 0x5E4327C8u, "Enus", "Rolls-Royce / Bentley" }, // windsor
    { 0x5EE005DAu, "Principe", "Ducati" }, // deveste
    { 0x5F2C43B9u, "Pfister", "Porsche" }, // astrale
    { 0x6068AD86u, "Vulcar", "Volvo" }, // fagaloa
    { 0x619C1B82u, "Vapid", "Ford" }, // imperator2
    { 0x61FE4D6Au, "BF", "Volkswagen" }, // weevil
    { 0x6210CBB0u, "Declasse", "Chevrolet" }, // RancherXL
    { 0x6290F15Bu, "MTL", "Mack" }, // pounder2
    { 0x6322B39Au, "Progen", "McLaren" }, // t20
    { 0x63ABADE7u, "Dinka", "Honda" }, // akuma
    { 0x64430650u, "Karin", "Toyota" }, // dilettante2
    { 0x64C1C8AAu, "Vulcar", "Volvo" }, // driftnebula
    { 0x64F49967u, "Declasse", "Chevrolet" }, // yosemite2
    { 0x65BDEBFCu, "Imponte", "Pontiac" }, // ruiner4
    { 0x665F785Du, "Albany", "Cadillac" }, // manana2
    { 0x669EB40Au, "Bravado", "Dodge" }, // monster3
    { 0x66B4FC45u, "Zirconium", "Saturn" }, // stratum
    { 0x67278B2Eu, "Dinka", "Honda" }, // driftrt3000
    { 0x678DD3EAu, "Bravado", "Dodge" }, // polgreenwood
    { 0x679450AFu, "Dewbauchee", "Aston Martin" }, // RapidGT2
    { 0x67B3F020u, "Bravado", "Dodge" }, // Bison3
    { 0x67D2B389u, "Benefactor", "Mercedes-Benz" }, // streiter
    { 0x67D52852u, "Vapid", "Ford" }, // slamvan6
    { 0x6827CF72u, "Brute", "GMC" }, // stockade
    { 0x6838FC1Du, "Overflod", "Koenigsegg" }, // entity3
    { 0x6882FA73u, "Dinka", "Honda" }, // enduro
    { 0x68A5D1EFu, "Ubermacht", "BMW" }, // cypher
    { 0x68FB5379u, "Dewbauchee", "Aston Martin" }, // rapidgt4
    { 0x690A4153u, "Declasse", "Chevrolet" }, // tornado3
    { 0x698521E3u, "Bravado", "Dodge" }, // dloader
    { 0x69F06B57u, "Albany", "Cadillac" }, // washington
    { 0x6ABDF65Eu, "Principe", "Ducati" }, // diablous2
    { 0x6B73A9BEu, "Bravado", "Dodge" }, // youga3
    { 0x6CBD1D6Du, "Western", "Cessna" }, // besra
    { 0x6D19CCBCu, "Vapid", "Ford" }, // peyote
    { 0x6D6F8F43u, "Dinka", "Honda" }, // thrust
    { 0x6DBD6C0Au, "Vapid", "Ford" }, // retinue
    { 0x6E8DA4F7u, "Weeny", "Mini" }, // issi7
    { 0x6EF89CCCu, "Shitzu", "Suzuki" }, // longfin
    { 0x6F039A67u, "Ubermacht", "BMW" }, // zion3
    { 0x6F946279u, "Declasse", "Chevrolet" }, // yosemite
    { 0x6FACDF31u, "Western", "Harley-Davidson" }, // ratbike
    { 0x6FD95F68u, "Brute", "GMC" }, // CAMPER
    { 0x6FF0F727u, "Gallivanter", "Land Rover" }, // baller3
    { 0x70241EEAu, "Ubermacht", "BMW" }, // niobe
    { 0x706E2B40u, "Dewbauchee", "Aston Martin" }, // SPECTER
    { 0x7074F39Du, "HVY", "Caterpillar / Oshkosh" }, // bulldozer
    { 0x707E63A4u, "Lampadati", "Maserati" }, // tropos
    { 0x70B8833Au, "Willard", "Buick" }, // polfaction2
    { 0x710A2B9Bu, "Declasse", "Chevrolet" }, // moonbeam2
    { 0x711D4738u, "BF", "Volkswagen" }, // dune3
    { 0x71CB2FFBu, "Cheval", "Holden" }, // fugitive
    { 0x71CBEA98u, "Vapid", "Ford" }, // gb200
    { 0x71D3B6F0u, "MTL", "Mack" }, // cerberus3
    { 0x72934BE4u, "Benefactor", "Mercedes-Benz" }, // schafter6
    { 0x72A4C31Eu, "Declasse", "Chevrolet" }, // stalion
    { 0x72A6CEBEu, "Annis", "Nissan" }, // driftzr350
    { 0x7341576Bu, "Declasse", "Chevrolet" }, // rancherxl2
    { 0x734C5E50u, "Bravado", "Dodge" }, // gauntlet4
    { 0x73920F8Eu, "MTL", "Mack" }, // firetruk
    { 0x7397224Cu, "Dewbauchee", "Aston Martin" }, // vagner
    { 0x73F4110Eu, "Maibatsu", "Mitsubishi" }, // mule4
    { 0x744CA80Du, "Brute", "GMC" }, // Taco
    { 0x75397001u, "Karin", "Toyota" }, // woodlander
    { 0x75599EA7u, "Pegassi", "Lamborghini" }, // pizzaboy
    { 0x767164D6u, "Pegassi", "Lamborghini" }, // osiris
    { 0x76BC2267u, "Dinka", "Honda" }, // chavosv6
    { 0x76D7C404u, "Western", "Harley-Davidson" }, // reever
    { 0x77934CEEu, "Western", "Harley-Davidson" }, // daemon
    { 0x779B4F2Du, "Declasse", "Chevrolet" }, // voodoo
    { 0x779F23AAu, "Albany", "Cadillac" }, // cavalcade
    { 0x7836CE2Fu, "Karin", "Toyota" }, // futo
    { 0x79178F0Au, "Vapid", "Ford" }, // retinue2
    { 0x794CB30Cu, "Pegassi", "Lamborghini" }, // esskey
    { 0x7980BDD5u, "Annis", "Nissan" }, // Euros
    { 0x798682A2u, "Declasse", "Chevrolet" }, // brutus3
    { 0x79C12D73u, "Invetero", "Corvette" }, // polcoquette4
    { 0x79DD18AEu, "HVY", "Caterpillar / Oshkosh" }, // menacer
    { 0x7A2EF5E4u, "Dewbauchee", "Aston Martin" }, // rapidgt3
    { 0x7A61B330u, "Vapid", "Ford" }, // Benson
    { 0x7B406EFBu, "Progen", "McLaren" }, // tyrus
    { 0x7B47A6A7u, "Albany", "Cadillac" }, // lurcher
    { 0x7B54A9D3u, "Pegassi", "Lamborghini" }, // oppressor2
    { 0x7B7E56F0u, "HVY", "Caterpillar / Oshkosh" }, // insurgent2
    { 0x7B8297C5u, "Bravado", "Dodge" }, // Bison2
    { 0x7B8AB45Fu, "Grotti", "Ferrari" }, // carbonizzare
    { 0x7D326F04u, "Toundra", "Alpine" }, // panthere
    { 0x7DE35E7Du, "MTL", "Mack" }, // Pounder
    { 0x7E8F677Fu, "Grotti", "Ferrari" }, // prototipo
    { 0x7F3415E3u, "Imponte", "Pontiac" }, // dukes3
    { 0x7F5C91F1u, "Obey", "Audi" }, // rocoto
    { 0x7F81A829u, "Declasse", "Chevrolet" }, // brutus
    { 0x806B9CC3u, "Western", "Harley-Davidson" }, // bagger
    { 0x809AA4CBu, "JoBuilt", "Peterbilt" }, // Phantom
    { 0x810369E2u, "HVY", "Caterpillar / Oshkosh" }, // dump
    { 0x8125BCF9u, "Nagasaki", "Kawasaki" }, // blazer
    { 0x81634188u, "Albany", "Cadillac" }, // manana
    { 0x817AFAADu, "Bravado", "Dodge" }, // gauntlet5
    { 0x8198AEDCu, "Overflod", "Koenigsegg" }, // entity2
    { 0x81A9CDDFu, "Willard", "Buick" }, // faction
    { 0x81BD2ED0u, "Mammoth", "Hummer" }, // avenger
    { 0x81E38F7Fu, "LCC", "Orange County Choppers" }, // avarus
    { 0x825A9F4Cu, "Vapid", "Ford" }, // guardian
    { 0x829A3C44u, "MTL", "Mack" }, // rallytruck
    { 0x82E47E85u, "BF", "Volkswagen" }, // club
    { 0x82E499FAu, "Grotti", "Ferrari" }, // stingergt
    { 0x83051506u, "Karin", "Toyota" }, // technical
    { 0x83070B62u, "Declasse", "Chevrolet" }, // impaler
    { 0x831A21D5u, "Imponte", "Pontiac" }, // Phoenix
    { 0x8408F33Au, "Grotti", "Ferrari" }, // gt500
    { 0x843B73DEu, "Stanley", "John Deere" }, // tractor2
    { 0x84F42E51u, "Canis", "Jeep" }, // MESA3
    { 0x8526E2F5u, "Vapid", "Ford" }, // slamvan4
    { 0x85A5B471u, "Maibatsu", "Mitsubishi" }, // Mule3
    { 0x85E8E76Bu, "Progen", "McLaren" }, // italigtb
    { 0x8612B64Bu, "Karin", "Toyota" }, // rebel2
    { 0x8644331Au, "Benefactor", "Mercedes-Benz" }, // bruiser3
    { 0x86618EDAu, "Albany", "Cadillac" }, // primo2
    { 0x866BCE26u, "Willard", "Buick" }, // faction3
    { 0x86CF7CDDu, "Declasse", "Chevrolet" }, // tornado4
    { 0x86FE0B60u, "Enus", "Rolls-Royce / Bentley" }, // cognoscenti
    { 0x871160F9u, "Vapid", "Ford" }, // youga5
    { 0x877358ADu, "Pfister", "Porsche" }, // comet3
    { 0x8911B9F5u, "Benefactor", "Mercedes-Benz" }, // feltzer2
    { 0x897AFC65u, "Benefactor", "Mercedes-Benz" }, // terbyte
    { 0x898ECCEAu, "Brute", "GMC" }, // boxville
    { 0x89BA59F5u, "Nagasaki", "Kawasaki" }, // havok
    { 0x89D676A1u, "Declasse", "Chevrolet" }, // driftl352
    { 0x8A63C7B9u, "Vapid", "Ford" }, // police4
    { 0x8B13F083u, "Dundreary", "Mercury / Lincoln" }, // stretch
    { 0x8B213907u, "Ocelot", "Jaguar" }, // formula2
    { 0x8B44BEFBu, "Invetero", "Corvette" }, // coquette5
    { 0x8BD565B8u, "Vapid", "Ford" }, // polcaracara
    { 0x8C2BD0DCu, "Imponte", "Pontiac" }, // nightshade
    { 0x8CB29A14u, "Dewbauchee", "Aston Martin" }, // RapidGT
    { 0x8CC51028u, "Classique", "Oldsmobile" }, // broadway
    { 0x8CF5CAE1u, "Enus", "Rolls-Royce / Bentley" }, // windsor2
    { 0x8D45DF49u, "Declasse", "Chevrolet" }, // impaler3
    { 0x8D4B7A8Au, "HVY", "Caterpillar / Oshkosh" }, // insurgent3
    { 0x8D780D37u, "Western", "Harley-Davidson" }, // policeb2
    { 0x8E08EC82u, "MTL", "Mack" }, // wastelander
    { 0x8E9254FBu, "Karin", "Toyota" }, // asterope
    { 0x8EF5E388u, "Declasse", "Chevrolet" }, // yosemite1500
    { 0x8F0E3594u, "Cheval", "Holden" }, // surge
    { 0x8F49AE28u, "Declasse", "Chevrolet" }, // brutus2
    { 0x8FB66F9Bu, "Declasse", "Chevrolet" }, // premier
    { 0x8FC3AADCu, "Albany", "Cadillac" }, // Emperor2
    { 0x8FC50F21u, "MTL", "Mack" }, // flatbed2
    { 0x8FD54EBBu, "Vom Feuer", "Heckler & Koch" }, // trailersmall2
    { 0x9114EADAu, "HVY", "Caterpillar / Oshkosh" }, // insurgent
    { 0x91373058u, "Annis", "Nissan" }, // zr350
    { 0x91673D0Eu, "Ubermacht", "BMW" }, // rhinehart
    { 0x91CA96EEu, "Pfister", "Porsche" }, // neon
    { 0x920016F1u, "Buckingham", "Bell" }, // volatus
    { 0x9229E4EBu, "Pegassi", "Lamborghini" }, // faggio
    { 0x92EF6E04u, "Pfister", "Porsche" }, // pfister811
    { 0x92F5024Eu, "Lampadati", "Maserati" }, // Novak
    { 0x93F09558u, "Western", "Harley-Davidson" }, // deathbike2
    { 0x94114926u, "Canis", "Jeep" }, // seminole2
    { 0x94204D89u, "Declasse", "Chevrolet" }, // asea
    { 0x9441D8D5u, "Declasse", "Chevrolet" }, // asea2
    { 0x9472CD24u, "Vapid", "Ford" }, // peyote2
    { 0x94B395C5u, "Bravado", "Dodge" }, // Gauntlet
    { 0x94DA98EFu, "Declasse", "Chevrolet" }, // tornado5
    { 0x95466BDBu, "Willard", "Buick" }, // faction2
    { 0x95F6A2C9u, "Pfister", "Porsche" }, // xtreme
    { 0x961AFEF7u, "Bravado", "Dodge" }, // rumpo2
    { 0x9628879Cu, "Declasse", "Chevrolet" }, // GRANGER
    { 0x96E0736Bu, "Declasse", "Chevrolet" }, // l35
    { 0x96E24857u, "Nagasaki", "Kawasaki" }, // microlight
    { 0x96E6932Du, "Dinka", "Honda" }, // driftjester
    { 0x973141FCu, "Declasse", "Chevrolet" }, // vigero2
    { 0x9734F3EAu, "Ocelot", "Jaguar" }, // penetrator
    { 0x97398A4Bu, "Dewbauchee", "Aston Martin" }, // SEVEN70
    { 0x97553C28u, "Karin", "Toyota" }, // everon
    { 0x97FA4F36u, "Declasse", "Chevrolet" }, // gburrito
    { 0x9804F4C7u, "Declasse", "Chevrolet" }, // impaler4
    { 0x98171BD3u, "Declasse", "Chevrolet" }, // burrito3
    { 0x98F65A5Eu, "Invetero", "Corvette" }, // coquette4
    { 0x991EFC04u, "Pfister", "Porsche" }, // comet6
    { 0x9A474B5Eu, "Kraken", "Triton" }, // avisa
    { 0x9A5B1DCCu, "JoBuilt", "Peterbilt" }, // Rubble
    { 0x9AE43988u, "Declasse", "Chevrolet" }, // drifttampa
    { 0x9AE6DDA1u, "Vapid", "Ford" }, // bullet
    { 0x9B065C9Eu, "Benefactor", "Mercedes-Benz" }, // bruiser2
    { 0x9B909C94u, "Declasse", "Chevrolet" }, // sabregt
    { 0x9BC400EFu, "Declasse", "Chevrolet" }, // driftyosemite
    { 0x9C32EB57u, "Vapid", "Ford" }, // police5
    { 0x9C5E5644u, "Buckingham", "Bell" }, // supervolito2
    { 0x9C669788u, "Dinka", "Honda" }, // double
    { 0x9CF21E0Fu, "BF", "Volkswagen" }, // dune
    { 0x9CFFFC56u, "Declasse", "Chevrolet" }, // mamba
    { 0x9D1D9872u, "Buckingham", "Bell" }, // conada2
    { 0x9D96B45Bu, "Vapid", "Ford" }, // radi
    { 0x9DAE1398u, "JoBuilt", "Peterbilt" }, // phantom2
    { 0x9EF6AEC7u, "Bravado", "Dodge" }, // poldorado
    { 0x9F04C481u, "Zirconium", "Saturn" }, // journey2
    { 0x9F3273F4u, "Annis", "Nissan" }, // driftremus
    { 0x9F4B77BEu, "Coil", "Tesla" }, // voltic
    { 0x9F6ED5A2u, "Vysser", "Spyker" }, // neo
    { 0xA0438767u, "Western", "Harley-Davidson" }, // nightblade
    { 0xA1355F67u, "Nagasaki", "Kawasaki" }, // blazer5
    { 0xA17C88F3u, "Ubermacht", "BMW" }, // sentinel5
    { 0xA1B3A871u, "Dinka", "Honda" }, // jester4
    { 0xA2073353u, "MTL", "Mack" }, // brickade2
    { 0xA29D6D10u, "Benefactor", "Mercedes-Benz" }, // feltzer3
    { 0xA29F78B0u, "Vapid", "Ford" }, // clique
    { 0xA31CB573u, "Declasse", "Chevrolet" }, // tornado6
    { 0xA34F2F09u, "Shitzu", "Suzuki" }, // driftkeitora
    { 0xA3FC0F4Du, "Bravado", "Dodge" }, // gresley
    { 0xA42FC3A5u, "Emperor", "Lexus" }, // vectre
    { 0xA4A4E453u, "Vapid", "Ford" }, // riata
    { 0xA4D99B7Du, "Coil", "Tesla" }, // raiden
    { 0xA4F52C13u, "Lampadati", "Maserati" }, // cinquemila
    { 0xA52F6866u, "Buckingham", "Gulfstream / Learjet" }, // alphaz1
    { 0xA5325278u, "Maibatsu", "Mitsubishi" }, // manchez
    { 0xA617546Cu, "Declasse", "Chevrolet" }, // tampa4
    { 0xA6297CC8u, "Karin", "Toyota" }, // futo2
    { 0xA703E4A9u, "Dinka", "Honda" }, // veto2
    { 0xA71D0D4Fu, "Pfister", "Porsche" }, // astron2
    { 0xA774B5A6u, "Benefactor", "Mercedes-Benz" }, // schafter3
    { 0xA7C9F9A7u, "Fathom", "Infiniti" }, // driftfr36
    { 0xA7CE1BC5u, "Coil", "Tesla" }, // brawler
    { 0xA7DCC35Cu, "Annis", "Nissan" }, // zr3803
    { 0xA7EDE74Du, "Vapid", "Ford" }, // stanier
    { 0xA8940CF3u, "RUNE", "Lada" }, // driftcheburek
    { 0xA8E38B01u, "Obey", "Audi" }, // ninef2
    { 0xA960B13Eu, "Maibatsu", "Mitsubishi" }, // sanchez2
    { 0xA988D3A2u, "Bollokan", "Hyundai" }, // prairie
    { 0xA9EC907Bu, "Pegassi", "Lamborghini" }, // ignus
    { 0xAA699BB6u, "Canis", "Jeep" }, // Bodhi2
    { 0xAC33179Cu, "Pegassi", "Lamborghini" }, // infernus2
    { 0xAC4E93C9u, "Western", "Harley-Davidson" }, // daemon2
    { 0xAC5DF515u, "Pegassi", "Lamborghini" }, // zentorno
    { 0xAD5E30D7u, "Western", "Harley-Davidson" }, // powersurge
    { 0xAD6065C0u, "Buckingham", "Gulfstream / Learjet" }, // pyro
    { 0xAE0A3D4Fu, "Vapid", "Ford" }, // dominator5
    { 0xAE12C99Cu, "Western", "Harley-Davidson" }, // deathbike3
    { 0xAE2BFE94u, "Karin", "Toyota" }, // kuruma
    { 0xAE2CC02Au, "Karin", "Toyota" }, // vivanite
    { 0xAED64A63u, "Vapid", "Ford" }, // chino2
    { 0xAF0B8D48u, "Lampadati", "Maserati" }, // tigon
    { 0xAF1FA439u, "Ubermacht", "BMW" }, // sentinel4
    { 0xAF599F01u, "Dinka", "Honda" }, // vindicator
    { 0xAF966F3Cu, "Vapid", "Ford" }, // caracara2
    { 0xAFBB2CA4u, "Declasse", "Chevrolet" }, // Burrito
    { 0xB1412350u, "Canis", "Jeep" }, // polterminus
    { 0xB1D80E06u, "BF", "Volkswagen" }, // Surfer2
    { 0xB1D95DA0u, "Grotti", "Ferrari" }, // cheetah
    { 0xB2A716A3u, "Dinka", "Honda" }, // jester
    { 0xB2CF7250u, "Buckingham", "Gulfstream / Learjet" }, // nimbus
    { 0xB2E046FBu, "Vapid", "Ford" }, // dominator6
    { 0xB2FE5CF9u, "Overflod", "Koenigsegg" }, // entityxf
    { 0xB3206692u, "Vulcar", "Volvo" }, // ingot
    { 0xB327FA66u, "Karin", "Toyota" }, // driftfuto2
    { 0xB328B188u, "Pegassi", "Lamborghini" }, // faggio3
    { 0xB44F0582u, "Nagasaki", "Kawasaki" }, // blazer3
    { 0xB472D2B5u, "Vapid", "Ford" }, // ellie
    { 0xB4F32118u, "Vapid", "Ford" }, // flashgt
    { 0xB52B5113u, "Benefactor", "Mercedes-Benz" }, // schafter2
    { 0xB581BF9Au, "Willard", "Buick" }, // eudora
    { 0xB5D306A4u, "Obey", "Audi" }, // tailgater2
    { 0xB5FCF74Eu, "Albany", "Cadillac" }, // emperor3
    { 0xB614798Du, "Benefactor", "Mercedes-Benz" }, // vorschlaghammer
    { 0xB6410173u, "Benefactor", "Mercedes-Benz" }, // dubsta3
    { 0xB67633E6u, "Bravado", "Dodge" }, // polgauntlet
    { 0xB6846A55u, "Annis", "Nissan" }, // le7b
    { 0xB779A091u, "Truffade", "Bugatti" }, // adder
    { 0xB79C1BF5u, "Buckingham", "Gulfstream / Learjet" }, // Shamal
    { 0xB79F589Eu, "Buckingham", "Gulfstream / Learjet" }, // luxor2
    { 0xB7D9F7F1u, "Declasse", "Chevrolet" }, // tampa3
    { 0xB802DD46u, "Karin", "Toyota" }, // Rebel
    { 0xB820ED5Eu, "Vapid", "Ford" }, // blade
    { 0xB8D657ADu, "Karin", "Toyota" }, // calico
    { 0xB8E2AE18u, "Ubermacht", "BMW" }, // zion2
    { 0xB9210FD0u, "Vapid", "Ford" }, // sandking
    { 0xB9CB3B69u, "Weeny", "Mini" }, // issi2
    { 0xBA5334ACu, "Pegassi", "Lamborghini" }, // toros
    { 0xBB6B404Fu, "Albany", "Cadillac" }, // primo
    { 0xBB78956Au, "Grotti", "Ferrari" }, // italirsx
    { 0xBBA2A2F7u, "HVY", "Caterpillar / Oshkosh" }, // scarab
    { 0xBC32A33Bu, "Fathom", "Infiniti" }, // fq2
    { 0xBC5DC07Eu, "Cheval", "Holden" }, // taipan
    { 0xBC7C0A00u, "Overflod", "Koenigsegg" }, // imorgon
    { 0xBC993509u, "Karin", "Toyota" }, // dilettante
    { 0xBCDE91F0u, "Vapid", "Ford" }, // minivan2
    { 0xBD1B39C3u, "Ubermacht", "BMW" }, // zion
    { 0xBE0E6126u, "Dinka", "Honda" }, // jester2
    { 0xBE11EFC6u, "Annis", "Nissan" }, // zr3802
    { 0xBE5E70B6u, "Annis", "Nissan" }, // minimus
    { 0xBF1691E0u, "Lampadati", "Maserati" }, // furoregt
    { 0xC0240885u, "Declasse", "Chevrolet" }, // tampa2
    { 0xC1632BEBu, "Maibatsu", "Mitsubishi" }, // Mule2
    { 0xC1A8A914u, "Vapid", "Ford" }, // slamtruck
    { 0xC1AE4D16u, "Pfister", "Porsche" }, // comet2
    { 0xC1CE1183u, "Dinka", "Honda" }, // marquis
    { 0xC1E908D2u, "Bravado", "Dodge" }, // banshee
    { 0xC247AEE5u, "BF", "Volkswagen" }, // surfer3
    { 0xC2974024u, "Speedophile", "Sea-Doo" }, // seashark
    { 0xC29F8F4Eu, "Albany", "Cadillac" }, // cavalcade3
    { 0xC397F748u, "Albany", "Cadillac" }, // buccaneer2
    { 0xC3D7C72Bu, "Western", "Harley-Davidson" }, // zombiea
    { 0xC3DDFDCEu, "Obey", "Audi" }, // tailgater
    { 0xC3F25753u, "Buckingham", "Gulfstream / Learjet" }, // howard
    { 0xC3F57329u, "Vapid", "Ford" }, // fmj2
    { 0xC3FBA120u, "HVY", "Caterpillar / Oshkosh" }, // cutter
    { 0xC4069F5Au, "Ubermacht", "BMW" }, // driftsentinel2
    { 0xC46B66C2u, "Annis", "Nissan" }, // eurosX32
    { 0xC4810400u, "Grotti", "Ferrari" }, // visione
    { 0xC4BB1908u, "BF", "Volkswagen" }, // weevil2
    { 0xC514AAE0u, "RUNE", "Lada" }, // cheburek
    { 0xC52C6B93u, "Vapid", "Ford" }, // dominator3
    { 0xC530D126u, "Ubermacht", "BMW" }, // driftsentinel
    { 0xC575DF11u, "Grotti", "Ferrari" }, // turismo2
    { 0xC58DA34Au, "Nagasaki", "Kawasaki" }, // dinghy5
    { 0xC5A12E61u, "Vapid", "Ford" }, // clique2
    { 0xC5DD6967u, "Western", "Cessna" }, // rogue
    { 0xC600F136u, "Vapid", "Ford" }, // firebolt
    { 0xC79C754Au, "Enus", "Rolls-Royce / Bentley" }, // paragon3
    { 0xC7E55211u, "Ocelot", "Jaguar" }, // locust
    { 0xC7FED061u, "Vapid", "Ford" }, // driftdominator10
    { 0xC8163646u, "Progen", "McLaren" }, // luiva
    { 0xC96B73D9u, "Vapid", "Ford" }, // dominator2
    { 0xC972A155u, "Dewbauchee", "Aston Martin" }, // champion
    { 0xC98BBAD6u, "Benefactor", "Mercedes-Benz" }, // glendale2
    { 0xC9CEAF06u, "Shitzu", "Suzuki" }, // pcj
    { 0xC9E8FF76u, "Declasse", "Chevrolet" }, // burrito2
    { 0xCA495705u, "Mammoth", "Hummer" }, // dodo
    { 0xCA62927Au, "Dundreary", "Mercury / Lincoln" }, // virgo2
    { 0xCAB6E261u, "Obey", "Audi" }, // tenf
    { 0xCABD11E8u, "Pegassi", "Lamborghini" }, // ruffian
    { 0xCADD5D2Du, "Pegassi", "Lamborghini" }, // bati2
    { 0xCB0E7CD9u, "Benefactor", "Mercedes-Benz" }, // schafter5
    { 0xCB642637u, "Vulcar", "Volvo" }, // nebula
    { 0xCC8A305Cu, "Gallivanter", "Land Rover" }, // baller8
    { 0xCCE5C8FAu, "Dinka", "Honda" }, // veto
    { 0xCCFCC32Fu, "Vapid", "Ford" }, // driftdominator9
    { 0xCD93A7DBu, "Vapid", "Ford" }, // monster
    { 0xCDC46B21u, "Brute", "GMC" }, // boxville6
    { 0xCE0B9F22u, "Dundreary", "Mercury / Lincoln" }, // landstalker2
    { 0xCE44C4B9u, "Lampadati", "Maserati" }, // komoda
    { 0xCE6B35A4u, "Albany", "Cadillac" }, // btype2
    { 0xCEC6B9B7u, "Declasse", "Chevrolet" }, // vigero
    { 0xCFB3870Cu, "Vapid", "Ford" }, // speedo
    { 0xCFCA3668u, "Gallivanter", "Land Rover" }, // Baller
    { 0xCFCFEB3Bu, "Mammoth", "Hummer" }, // patriot
    { 0xD039510Bu, "MTL", "Mack" }, // cerberus
    { 0xD0EB2BE5u, "Albany", "Cadillac" }, // cavalcade2
    { 0xD138A6BBu, "HVY", "Caterpillar / Oshkosh" }, // Mixer
    { 0xD1AD4937u, "Obey", "Audi" }, // omnis
    { 0xD1E0B7D7u, "Vapid", "Ford" }, // poldominator10
    { 0xD235A4A6u, "Declasse", "Chevrolet" }, // draugur
    { 0xD2389392u, "Bravado", "Dodge" }, // dorado
    { 0xD2D5E00Eu, "Pegassi", "Lamborghini" }, // fcr2
    { 0xD2F77E37u, "Vapid", "Ford" }, // imperator3
    { 0xD3046147u, "Lampadati", "Maserati" }, // corsita
    { 0xD35698EFu, "Mammoth", "Hummer" }, // mogul
    { 0xD36A4B44u, "Canis", "Jeep" }, // mesa2
    { 0xD37B7976u, "Benefactor", "Mercedes-Benz" }, // schwarzer
    { 0xD3D366B1u, "Karin", "Toyota" }, // asterope2
    { 0xD556917Cu, "Bravado", "Dodge" }, // monster5
    { 0xD6FB0F30u, "Vapid", "Ford" }, // dominator4
    { 0xD7278283u, "Albany", "Cadillac" }, // emperor
    { 0xD756460Cu, "Albany", "Cadillac" }, // buccaneer
    { 0xD757D97Du, "Pegassi", "Lamborghini" }, // zorrusso
    { 0xD7C56D39u, "BF", "Volkswagen" }, // raptor
    { 0xD80F4A44u, "Mammoth", "Hummer" }, // patriot3
    { 0xD86A0247u, "Benefactor", "Mercedes-Benz" }, // krieger
    { 0xD876DBE2u, "Vapid", "Ford" }, // trophytruck2
    { 0xD8A914D3u, "Bravado", "Dodge" }, // banshee3
    { 0xD8FD24D1u, "Albany", "Cadillac" }, // brigham
    { 0xD9F0503Du, "Declasse", "Chevrolet" }, // scramjet
    { 0xDA288376u, "Principe", "Ducati" }, // nemesis
    { 0xDA5819A3u, "Dewbauchee", "Aston Martin" }, // massacro2
    { 0xDA5EC7DAu, "Maibatsu", "Mitsubishi" }, // penumbra2
    { 0xDAC67112u, "Ocelot", "Jaguar" }, // jackal
    { 0xDB0C9B04u, "Bravado", "Dodge" }, // buffalo4
    { 0xDB20A373u, "Western", "Harley-Davidson" }, // wolfsbane
    { 0xDB4388E4u, "Speedophile", "Sea-Doo" }, // seashark2
    { 0xDBA9DBFCu, "Pegassi", "Lamborghini" }, // vortex
    { 0xDBF2D57Au, "Enus", "Rolls-Royce / Bentley" }, // cognoscenti2
    { 0xDC19D101u, "Albany", "Cadillac" }, // btype3
    { 0xDC434E51u, "Vapid", "Ford" }, // Sadler
    { 0xDCBC1C3Bu, "Dinka", "Honda" }, // blista3
    { 0xDCBCBE48u, "Ocelot", "Jaguar" }, // f620
    { 0xDCE1D9F7u, "Bravado", "Dodge" }, // ratloader2
    { 0xDD71BFEBu, "HVY", "Caterpillar / Oshkosh" }, // scarab3
    { 0xDE05FB87u, "Western", "Harley-Davidson" }, // zombieb
    { 0xDE3D9D22u, "Annis", "Nissan" }, // elegy2
    { 0xE00BADABu, "Vapid", "Ford" }, // ratel
    { 0xE18195B2u, "Ubermacht", "BMW" }, // oracle2
    { 0xE1C03AB0u, "Benefactor", "Mercedes-Benz" }, // schlagen
    { 0xE1E2E6D7u, "Obey", "Audi" }, // omnisegt
    { 0xE2504942u, "Albany", "Cadillac" }, // virgo
    { 0xE33A477Bu, "Progen", "McLaren" }, // italigtb2
    { 0xE3788BB1u, "Declasse", "Chevrolet" }, // impaler5
    { 0xE384DD25u, "Buckingham", "Bell" }, // conada
    { 0xE43C11E5u, "Fathom", "Infiniti" }, // fr36
    { 0xE478B977u, "Declasse", "Chevrolet" }, // tahoma
    { 0xE505CF99u, "Dinka", "Honda" }, // rt3000
    { 0xE550775Bu, "Enus", "Rolls-Royce / Bentley" }, // paragon
    { 0xE5B3ACA1u, "Vapid", "Ford" }, // dominator9
    { 0xE5BA6858u, "Nagasaki", "Kawasaki" }, // blazer4
    { 0xE62B361Bu, "Pegassi", "Lamborghini" }, // monroe
    { 0xE6401328u, "Benefactor", "Mercedes-Benz" }, // xls2
    { 0xE644E480u, "Benefactor", "Mercedes-Benz" }, // panto
    { 0xE68D8190u, "Mammoth", "Hummer" }, // avenger3
    { 0xE6E967F8u, "Mammoth", "Hummer" }, // patriot2
    { 0xE78CC3D9u, "Ubermacht", "BMW" }, // revolter
    { 0xE7D2A16Eu, "Nagasaki", "Kawasaki" }, // shotaro
    { 0xE8039FA7u, "Shitzu", "Suzuki" }, // keitora
    { 0xE80F67EEu, "Declasse", "Chevrolet" }, // stalion2
    { 0xE810B452u, "Annis", "Nissan" }, // drifthardy
    { 0xE882E5F6u, "Benefactor", "Mercedes-Benz" }, // dubsta2
    { 0xE8983F9Fu, "Western", "Cessna" }, // seabreeze
    { 0xE8A8BA94u, "Lampadati", "Maserati" }, // viseris
    { 0xE8A8BDA8u, "Lampadati", "Maserati" }, // felon
    { 0xE9805550u, "Maibatsu", "Mitsubishi" }, // penumbra
    { 0xE99011C2u, "Overflod", "Koenigsegg" }, // tyrant
    { 0xEA61C64Au, "Dinka", "Honda" }, // driftjester3
    { 0xEA6A047Fu, "Annis", "Nissan" }, // hellion
    { 0xEA76439Fu, "Bravado", "Dodge" }, // driftgauntlet4
    { 0xEA9F9B38u, "Ubermacht", "BMW" }, // sentinel6
    { 0xEAFA7EB7u, "Buckingham", "Bell" }, // cargobob5
    { 0xEB298297u, "BF", "Volkswagen" }, // bifta
    { 0xEB70965Fu, "Dinka", "Honda" }, // blista
    { 0xEBC24DF2u, "Buckingham", "Bell" }, // swift
    { 0xEC3E3404u, "Grotti", "Ferrari" }, // italigto
    { 0xEC8F7094u, "Imponte", "Pontiac" }, // dukes2
    { 0xECA6B6A3u, "Annis", "Nissan" }, // s80
    { 0xED552C74u, "Overflod", "Koenigsegg" }, // autarch
    { 0xED762D49u, "Speedophile", "Sea-Doo" }, // seashark3
    { 0xED7EADA4u, "Vapid", "Ford" }, // minivan
    { 0xEDC6F847u, "MTL", "Mack" }, // brickade
    { 0xEDD516C6u, "Bravado", "Dodge" }, // buffalo
    { 0xEE6024BCu, "Karin", "Toyota" }, // sultanrs
    { 0xEE6F8F79u, "Dinka", "Honda" }, // postlude
    { 0xEEA75E63u, "Karin", "Toyota" }, // sultan3
    { 0xEF2295C9u, "Shitzu", "Suzuki" }, // Suntrap
    { 0xF06C29C7u, "Declasse", "Chevrolet" }, // granger2
    { 0xF0C2A91Fu, "Shitzu", "Suzuki" }, // hakuchou2
    { 0xF1B44F44u, "Principe", "Ducati" }, // diablous
    { 0xF26CEFF9u, "Imponte", "Pontiac" }, // ruiner
    { 0xF2AE3F81u, "Overflod", "Koenigsegg" }, // pipistrello
    { 0xF330CB6Au, "Dinka", "Honda" }, // jester3
    { 0xF337AB36u, "Brute", "GMC" }, // stockade3
    { 0xF376F1E6u, "Vapid", "Ford" }, // winky
    { 0xF38C4245u, "Ocelot", "Jaguar" }, // jugular
    { 0xF52D92EEu, "Karin", "Toyota" }, // driftfuto
    { 0xF55D2F7Au, "Declasse", "Chevrolet" }, // impaler6
    { 0xF62446BAu, "Pegassi", "Lamborghini" }, // torero2
    { 0xF683EACAu, "LCC", "Orange County Choppers" }, // innovation
    { 0xF771092Eu, "Benefactor", "Mercedes-Benz" }, // driftvorschlag
    { 0xF77ADE32u, "Dewbauchee", "Aston Martin" }, // massacro
    { 0xF79A00F7u, "Shitzu", "Suzuki" }, // Vader
    { 0xF82BC92Eu, "Karin", "Toyota" }, // everon2
    { 0xF84B48D1u, "JoBuilt", "Peterbilt" }, // Phantom4
    { 0xF8AB457Bu, "Grotti", "Ferrari" }, // turismo3
    { 0xF8C2E0E7u, "Canis", "Jeep" }, // kamacho
    { 0xF8D48E7Au, "Zirconium", "Saturn" }, // journey
    { 0xF8DE29A8u, "Brute", "GMC" }, // pony
    { 0xF92AEC4Du, "Benefactor", "Mercedes-Benz" }, // limo2
    { 0xF9300CC5u, "Pegassi", "Lamborghini" }, // bati
    { 0xF9E67C05u, "Mammoth", "Hummer" }, // squaddie
    { 0xFAAD85EEu, "Lampadati", "Maserati" }, // felon2
    { 0xFBDEAC32u, "Mammoth", "Hummer" }, // avenger4
    { 0xFC2E479Au, "Dinka", "Honda" }, // kanjosj
    { 0xFCC2F483u, "Canis", "Jeep" }, // freecrawler
    { 0xFD128DFDu, "Declasse", "Chevrolet" }, // vamos
    { 0xFD231729u, "Nagasaki", "Kawasaki" }, // blazer2
    { 0xFD546A18u, "Vapid", "Ford" }, // speedo5
    { 0xFDAEBF27u, "Vapid", "Ford" }, // aleutian
    { 0xFE141DA6u, "Bravado", "Dodge" }, // halftrack
    { 0xFE5F0722u, "Western", "Harley-Davidson" }, // deathbike
    { 0xFEFD644Fu, "Bravado", "Dodge" }, // bison
    { 0xFF22D208u, "Dundreary", "Mercury / Lincoln" }, // regina
    { 0xFF5968CDu, "Benefactor", "Mercedes-Benz" }, // lm87
    { 0xFFB15B5Eu, "Dewbauchee", "Aston Martin" }, // exemplar
};
