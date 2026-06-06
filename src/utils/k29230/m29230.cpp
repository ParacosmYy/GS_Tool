#include "k29230/m29230.h"
QVector<double> m29230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
