#include "i9588/m9588.h"
QVector<double> m9588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
