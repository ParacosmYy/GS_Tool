#include "m30072/m30072.h"
QVector<double> m30072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
