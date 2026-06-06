#include "i12488/m12488.h"
QVector<double> m12488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
