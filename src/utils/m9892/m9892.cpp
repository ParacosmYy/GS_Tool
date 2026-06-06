#include "m9892/m9892.h"
QVector<double> m9892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
