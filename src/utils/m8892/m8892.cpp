#include "m8892/m8892.h"
QVector<double> m8892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
