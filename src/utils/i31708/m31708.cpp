#include "i31708/m31708.h"
QVector<double> m31708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
