#include "i12588/m12588.h"
QVector<double> m12588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
