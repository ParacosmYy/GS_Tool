#include "t7879/m7879.h"
QVector<double> m7879::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
