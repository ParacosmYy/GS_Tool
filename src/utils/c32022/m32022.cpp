#include "c32022/m32022.h"
QVector<double> m32022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
