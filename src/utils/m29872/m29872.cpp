#include "m29872/m29872.h"
QVector<double> m29872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
