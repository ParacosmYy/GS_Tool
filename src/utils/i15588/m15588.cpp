#include "i15588/m15588.h"
QVector<double> m15588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
