#include "a30100/m30100.h"
QVector<double> m30100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
