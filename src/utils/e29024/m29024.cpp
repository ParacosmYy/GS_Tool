#include "e29024/m29024.h"
QVector<double> m29024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
