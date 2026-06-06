#include "t29999/m29999.h"
QVector<double> m29999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
