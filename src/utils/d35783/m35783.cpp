#include "d35783/m35783.h"
QVector<double> m35783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
