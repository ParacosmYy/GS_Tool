#include "t7999/m7999.h"
QVector<double> m7999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
