#include "a9460/m9460.h"
QVector<double> m9460::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
