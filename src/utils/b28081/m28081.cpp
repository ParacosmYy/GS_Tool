#include "b28081/m28081.h"
QVector<double> m28081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
