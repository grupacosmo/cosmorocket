import struct
import sys
import struct

folder = sys.argv[1]
PRESSURE_TEMP_FILENAME = 'PRESSURE_TEMP.BIN'
ACCELERATION_FILENAME = 'ACCELERATION.BIN'
ANGULAR_RATE_FILENAME = 'ANGULAR_RATE.BIN'
GPS_FILENAME = 'GPS.BIN'
PRESSURE_TEMP_ROW_SIZE = 20
ACCELERATION_ROW_SIZE = 6
ANGULAR_RATE_ROW_SIZE = 6

# based on C:
# float_t lsm6dso_from_fs16_to_mg(int16_t lsb)
# {
#   return ((float_t)lsb) * 0.488f;
# }
# lsm6dso32 is using 2x higher range than lsm6dso
def lsm6dso_from_fs32_to_mg(lsb):
    return float(lsb) * 0.976

# based on C:
# float_t lsm6dso_from_fs2000_to_mdps(int16_t lsb)
# {
#   return ((float_t)lsb) * 70.0f;
# }
def lsm6dso_from_fs2000_to_mdps(lsb):
    return float(lsb) * 70.0

def process_pressure_temp():
    with open(folder + '/' + PRESSURE_TEMP_FILENAME, 'rb') as input_file:
        with open(folder + '/' + PRESSURE_TEMP_FILENAME.replace('BIN', 'TXT'), 'w') as output_file:
            output_file.write('TIME_SINCE_LAUNCH\tAIR_PRESSURE\tHUMIDITY\tTEMPERATURE\n')
            while True:
                row = input_file.read(PRESSURE_TEMP_ROW_SIZE)
                if len(row) != PRESSURE_TEMP_ROW_SIZE:
                    break
                (timeMicros, pressure, humidity, temperature) = struct.unpack('qfff', row)
                output_file.write(
                    f'{timeMicros / 1000000:.6f}\t'
                    f'{pressure:.1f}\t'
                    f'{humidity:.3f}'
                    f'\t{temperature:.2f}\n')

def process_acceleration():
    with open(folder + '/' + ACCELERATION_FILENAME, 'rb') as input_file:
        with open(folder + '/' + ACCELERATION_FILENAME.replace('BIN', 'TXT'), 'w') as output_file:
            output_file.write('X[milli-Gs]\tY[milli-Gs]\tZ[milli-Gs]\n')
            while True:
                row = input_file.read(ACCELERATION_ROW_SIZE)
                if len(row) != ACCELERATION_ROW_SIZE:
                    break
                (x, y, z) = struct.unpack('hhh', row)
                output_file.write(
                    f'{lsm6dso_from_fs32_to_mg(x):.2f}\t'
                    f'{lsm6dso_from_fs32_to_mg(y):.2f}\t'
                    f'{lsm6dso_from_fs32_to_mg(z):.2f}\n')

def process_angular_rate():
    with open(folder + '/' + ANGULAR_RATE_FILENAME, 'rb') as input_file:
        with open(folder + '/' + ANGULAR_RATE_FILENAME.replace('BIN', 'TXT'), 'w') as output_file:
            output_file.write('X[dps]\tY[dps]\tZ[dps]\n')
            while True:
                row = input_file.read(ANGULAR_RATE_ROW_SIZE)
                if len(row) != ANGULAR_RATE_ROW_SIZE:
                    break
                (x, y, z) = struct.unpack('hhh', row)
                output_file.write(
                    f'{lsm6dso_from_fs2000_to_mdps(x)/1000}\t'
                    f'{lsm6dso_from_fs2000_to_mdps(y)/1000}\t'
                    f'{lsm6dso_from_fs2000_to_mdps(z)/1000}\n')

def process_gps():
    with open(folder + '/' + GPS_FILENAME, 'rb') as input_file:
        with open(folder + '/' + GPS_FILENAME.replace('BIN', 'TXT'), 'w') as output_file:
            output_file.write('TIME_SINCE_LAUNCH\tGPS_DATA\n')
            while True:
                timestamp_index = input_file.read(8 + 1)
                if len(timestamp_index) != 8 + 1:
                    break
                (timestamp, index) = struct.unpack('qB', timestamp_index)
                decoded = ''
                match index:
                    case 0:
                        decoded = process_gps_sentence_gga(input_file)
                    case 1:
                        decoded = process_gps_sentence_rmc(input_file)
                    case 2:
                        decoded = process_gps_sentence_gll(input_file)
                    case 3:
                        decoded = process_gps_sentence_vtg(input_file)
                output_file.write(
                    f'{timestamp / 1000000:.6f}\t'
                    f'{decoded}\n')

def process_gps_sentence_gga(input_file):
    raw = input_file.read(88)
    if (len(raw) != 88):
        return ''
    (_, #type
    hours, minutes, seconds, microseconds,
    latitude, _,
    longitude, _,
    fix_quality,
    satellites_tracked,
    hdop, hdop_scale,
    altitude, altitude_scale, altitude_units,
    height, height_scale, height_units,
    dgps_age, dgps_age_scale) = struct.unpack(
        'Qiiiiiiiiiiiiiiciicii',
        raw
    )

    latitude = str(latitude)
    longitude = str(longitude)

    return (f'{hours:02d}:{minutes:02d}:{seconds:02d}.{microseconds // 100000}  '
    f'coords: {latitude[:2] + '.' + latitude[2:]} '
    f'{longitude[:2] + '.' + longitude[2:]}  '
    f'fix quality: {fix_quality}  '
    f'satellites tracked: {satellites_tracked}  '
    f'hdop: {hdop / hdop_scale}  '
    f'altitude: {altitude / altitude_scale}{altitude_units.decode('utf-8')}  '
    f'height: {height / height_scale}{height_units.decode('utf-8')}  '
    f'dgps age: {max(1, dgps_age)}')

def process_gps_sentence_rmc(input_file):
    raw = input_file.read(80)
    if (len(raw) != 80):
        return ''
    (_, #type
    hours, minutes, seconds, microseconds,
    valid,
    latitude, _,
    longitude, _,
    speed, speed_scale,
    course, course_scale,
    day, month, year,
    variation, variation_scale) = struct.unpack(
        'Qiiiiiiiiiiiiiiiiii',
        raw
    )

    latitude = str(latitude)
    longitude = str(longitude)

    return (f'{hours:02d}:{minutes:02d}:{seconds:02d}.{microseconds // 100000}  '
    f'coords: {latitude[:2] + '.' + latitude[2:]} '
    f'{longitude[:2] + '.' + longitude[2:]}  '
    f'date: {day:02d}-{month:02d}-{year:02d}  '
    f'valid: {valid}  '
    f'speed: {speed / speed_scale}  '
    f'course: {course / max(1, course_scale)}  '
    f'variation: {variation / max(1, variation_scale)}')

def process_gps_sentence_gll(input_file):
    raw = input_file.read(44)
    if (len(raw) != 44):
        return ''
    (_, #type
    latitude, _,
    longitude, _,
    hours, minutes, seconds, microseconds,
    status,
    mode,
    _, _ #padding
    ) = struct.unpack(
        'Qiiiiiiiicccc',
        raw
    )

    latitude = str(latitude)
    longitude = str(longitude)

    return (f'{hours:02d}:{minutes:02d}:{seconds:02d}.{microseconds // 100000}  '
    f'coords: {latitude[:2] + '.' + latitude[2:]} '
    f'{longitude[:2] + '.' + longitude[2:]}  '
    f'status: {status.decode('utf-8')}  '
    f'mode: {mode.decode('utf-8')}')

def process_gps_sentence_vtg(input_file):
    raw = input_file.read(44)
    if (len(raw) != 44):
        return ''
    (_, #type
    true_track_degrees, true_track_degrees_scale,
    magnetic_track_degrees, magnetic_track_degrees_scale,
    speed_knots, speed_knots_scale,
    speed_kph, speed_kph_scale,
    faa_mode) = struct.unpack(
        'Qiiiiiiiii',
        raw
    )

    # MINMEA_FAA_MODE_AUTONOMOUS = 'A',
    # MINMEA_FAA_MODE_DIFFERENTIAL = 'D',
    # MINMEA_FAA_MODE_ESTIMATED = 'E',
    # MINMEA_FAA_MODE_MANUAL = 'M',
    # MINMEA_FAA_MODE_SIMULATED = 'S',
    # MINMEA_FAA_MODE_NOT_VALID = 'N',
    # MINMEA_FAA_MODE_PRECISE = 'P',

    return (f'true_track_degrees: {true_track_degrees / max(1, true_track_degrees_scale)}  '
    f'magnetic_track_degrees: {magnetic_track_degrees / max(1, magnetic_track_degrees_scale)}  '
    f'speed_knots: {speed_knots / speed_knots_scale}  '
    f'speed_kph: {speed_kph / speed_kph_scale}  '
    f'faa mode: {chr(faa_mode)}')

process_pressure_temp();
process_acceleration();
process_angular_rate();
process_gps();
