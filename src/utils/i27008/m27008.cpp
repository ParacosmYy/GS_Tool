#include "i27008/m27008.h"
QVector<double> m27008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
