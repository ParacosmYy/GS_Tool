#include "l24591/m24591.h"
QVector<double> m24591::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
