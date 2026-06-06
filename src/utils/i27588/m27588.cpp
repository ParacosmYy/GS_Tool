#include "i27588/m27588.h"
QVector<double> m27588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
