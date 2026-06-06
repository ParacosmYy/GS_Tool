#include "p25835/m25835.h"
QVector<double> m25835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
