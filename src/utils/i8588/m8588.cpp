#include "i8588/m8588.h"
QVector<double> m8588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
