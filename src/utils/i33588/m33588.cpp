#include "i33588/m33588.h"
QVector<double> m33588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
