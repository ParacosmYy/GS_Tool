#include "m19052/m19052.h"
QVector<double> m19052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
