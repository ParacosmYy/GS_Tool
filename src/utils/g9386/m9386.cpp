#include "g9386/m9386.h"
QVector<double> m9386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
