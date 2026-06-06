#include "q29016/m29016.h"
QVector<double> m29016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
