#include "g19306/m19306.h"
QVector<double> m19306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
