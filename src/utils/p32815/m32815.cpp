#include "p32815/m32815.h"
QVector<double> m32815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
