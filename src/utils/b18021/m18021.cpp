#include "b18021/m18021.h"
QVector<double> m18021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
