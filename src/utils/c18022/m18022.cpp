#include "c18022/m18022.h"
QVector<double> m18022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
