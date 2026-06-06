#include "p32315/m32315.h"
QVector<double> m32315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
