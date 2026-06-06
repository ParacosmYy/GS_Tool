#include "g24706/m24706.h"
QVector<double> m24706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
