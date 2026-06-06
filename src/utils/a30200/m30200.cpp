#include "a30200/m30200.h"
QVector<double> m30200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
