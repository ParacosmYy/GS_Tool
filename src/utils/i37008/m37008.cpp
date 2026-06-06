#include "i37008/m37008.h"
QVector<double> m37008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
