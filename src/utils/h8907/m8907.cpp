#include "h8907/m8907.h"
QVector<double> m8907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
