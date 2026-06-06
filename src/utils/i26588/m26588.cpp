#include "i26588/m26588.h"
QVector<double> m26588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
