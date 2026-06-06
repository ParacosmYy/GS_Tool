#include "d34583/m34583.h"
QVector<double> m34583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
