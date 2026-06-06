#include "i17708/m17708.h"
QVector<double> m17708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
