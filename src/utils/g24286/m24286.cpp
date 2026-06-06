#include "g24286/m24286.h"
QVector<double> m24286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
