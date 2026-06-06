#include "c8022/m8022.h"
QVector<double> m8022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
