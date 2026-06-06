#include "i10588/m10588.h"
QVector<double> m10588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
