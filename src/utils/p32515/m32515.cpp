#include "p32515/m32515.h"
QVector<double> m32515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
