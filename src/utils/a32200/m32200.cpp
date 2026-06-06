#include "a32200/m32200.h"
QVector<double> m32200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
